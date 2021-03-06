/* This is targetgroup.cpp and is part of CANDOCK
 * Copyright (c) 2016-2019 Chopra Lab at Purdue University, 2013-2016 Janez Konc at National Institute of Chemistry and Samudrala Group at University of Washington
 *
 * This program is free for educational and academic use
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "candock/program/targetgroup.hpp"
#include "statchem/fileio/fileout.hpp"
#include "statchem/fileio/inout.hpp"
#include "statchem/helper/logger.hpp"

using namespace std;

namespace candock {
namespace Program {

using namespace statchem;

TargetGroup::TargetGroup(const std::string& input_name) {
    // User has given blank option (ussual for case of antitargets)
    if (input_name.empty()) {
        return;
    }

    for (const auto& a : fileio::files_matching_pattern(input_name, ".pdb")) {
        __targets.push_back(new Target(a));
    }
}

TargetGroup::~TargetGroup() {
    for (Target* target : __targets) {
        delete target;
    }
    __targets.clear();
}

void TargetGroup::dock_fragments(const FragmentLigands& ligands) {
    for (auto& target : __targets) {
        target->dock_fragments(ligands);
    }
}

void TargetGroup::dock_ligands(const FragmentLigands& ligands) {
    for (auto& target : __targets) {
        target->link_fragments(ligands);
    }
}

std::multiset<std::string> TargetGroup::determine_overlapping_seeds(
    const int max_seeds, const int number_of_occurances) const {
    std::multiset<std::string> good_seed_list;

    for (auto& a : __targets) {
        auto result = a->get_seeds().get_best_seeds();

        if (max_seeds != -1 && static_cast<size_t>(max_seeds) < result.size())
            result.resize(max_seeds);

        for (auto& b : result) good_seed_list.insert(b.second);
    }

    for (auto c = good_seed_list.cbegin(); c != good_seed_list.cend();) {
        if (static_cast<int>(good_seed_list.count(*c)) < number_of_occurances) {
            c = good_seed_list.erase(c);
        } else {
            ++c;
        }
    }

    return good_seed_list;
}

std::set<std::string> TargetGroup::determine_non_overlapping_seeds(
    const TargetGroup& antitargets) {
    set<string> solo_target_seeds;
    const vector<string>& forced_seeds = cmdl.get_string_vector("force_seed");

    if (forced_seeds.size() != 0 && forced_seeds[0] != "off") {
        std::copy(forced_seeds.begin(), forced_seeds.end(),
                  std::inserter(solo_target_seeds, solo_target_seeds.end()));
    } else {
        log_step << "Determining the best seeds to add" << endl;
        multiset<string> target_seeds =
            determine_overlapping_seeds(cmdl.get_int_option("seeds_to_add"),
                                        cmdl.get_int_option("seeds_till_good"));
        multiset<string> atarget_seeds =
            antitargets.determine_overlapping_seeds(
                cmdl.get_int_option("seeds_to_avoid"),
                cmdl.get_int_option("seeds_till_bad"));

        std::set_difference(
            target_seeds.begin(), target_seeds.end(), atarget_seeds.begin(),
            atarget_seeds.end(),
            std::inserter(solo_target_seeds, solo_target_seeds.end()));
    }

    return solo_target_seeds;
}

void TargetGroup::make_scaffolds(const TargetGroup& antitargets,
                                 FragmentLigands& ligands) {
    set<string> seeds_to_add = determine_non_overlapping_seeds(antitargets);

    std::stringstream used_seeds;
    for (auto& s : seeds_to_add) {
        used_seeds << s << endl;
    }

    fileio::output_file(used_seeds.str(), "new_scaffold_seeds.lst");

    molib::Molecules all_designs;

    log_step << "Starting iteration #0 (making a scaffold)" << endl;
    const string design_file = "designed_0.pdb";
    if (fileio::file_size(design_file)) {
        log_note
            << design_file
            << " found -- skipping generation of new designs this iteration"
            << endl;
        parser::FileParser dpdb(design_file, parser::all_models);
        molib::Molecules designs;
        dpdb.parse_molecule(designs);

        all_designs.add(designs);
    } else {
        for (auto& target : __targets) {
            target->make_scaffolds(seeds_to_add, all_designs);
        }
    }

    all_designs.compute_hydrogen()
        .compute_bond_order()
        .compute_bond_gaff_type()
        .refine_idatm_type()
        .erase_hydrogen()    // needed because refine changes connectivities
        .compute_hydrogen()  // needed because refine changes connectivities
        .compute_ring_type()
        .compute_gaff_type()
        .compute_rotatable_bonds()  // relies on hydrogens being assigned
        .erase_hydrogen()
        .compute_overlapping_rigid_segments(cmdl.get_string_option("seeds"));

    fileio::output_file(all_designs, design_file);

    std::stringstream ss;
    for (const auto& m : all_designs) {
        fileio::print_mol2(ss, m);
    }
    fileio::output_file(ss.str(), "designed_0.mol2");

    if (fileio::file_size("design_seeds.pdb"))
        ligands.add_seeds_from_file("design_seeds.pdb");
    ligands.add_seeds_from_molecules(all_designs);
    ligands.write_seeds_to_file("design_seeds.pdb");

    for (auto& target : __targets) {
        target->dock_fragments(ligands);
        target->link_fragments(all_designs);
    }
}

void TargetGroup::design_ligands(const TargetGroup& antitargets,
                                 FragmentLigands& ligands) {
    set<string> seeds_to_add = determine_non_overlapping_seeds(antitargets);

    std::stringstream used_seeds;
    for (auto& s : seeds_to_add) {
        used_seeds << s << endl;
    }

    fileio::output_file(used_seeds.str(), "lead_optimization_seeds.lst");

    int n = 0;
    while (true) {  // Probably a bad idea

        log_step << "Starting design iteration #" << ++n << endl;

        string design_file = "designed_" + std::to_string(n) + ".pdb";

        molib::Molecules all_designs;

        if (fileio::file_size(design_file)) {
            log_note
                << design_file
                << " found -- skipping generation of new designs this iteration"
                << endl;
            parser::FileParser dpdb(design_file, parser::all_models);
            molib::Molecules designs;
            dpdb.parse_molecule(designs);

            all_designs.add(designs);
        } else {
            for (auto& target : __targets) {
                target->design_ligands(seeds_to_add, all_designs);
            }
        }

        if (all_designs.size() == 0) {
            log_step << "No new designs, exiting" << endl;
            return;
        }

        all_designs.compute_hydrogen()
            .compute_bond_order()
            .compute_bond_gaff_type()
            .refine_idatm_type()
            .erase_hydrogen()    // needed because refine changes connectivities
            .compute_hydrogen()  // needed because refine changes connectivities
            .compute_ring_type()
            .compute_gaff_type()
            .compute_rotatable_bonds()  // relies on hydrogens being assigned
            .erase_hydrogen()
            .compute_overlapping_rigid_segments(
                cmdl.get_string_option("seeds"));

        fileio::output_file(all_designs, design_file);

        std::stringstream ss;
        for (const auto& m : all_designs) {
            fileio::print_mol2(ss, m);
        }
        fileio::output_file(ss.str(), "designed_" + std::to_string(n) + ".mol2");

        if (fileio::file_size("design_seeds.pdb"))
            ligands.add_seeds_from_file("design_seeds.pdb");
        ligands.add_seeds_from_molecules(all_designs);
        ligands.write_seeds_to_file("design_seeds.pdb");

        for (auto& target : __targets) {
            target->dock_fragments(ligands);
            target->link_fragments(all_designs);
        }
    }
}
}
}
