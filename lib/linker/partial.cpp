/* This is partial.cpp and is part of CANDOCK
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

#include "candock/linker/partial.hpp"
#include <algorithm>
#include <map>
#include <queue>
#include "statchem/graph/mcqd.hpp"
#include "statchem/helper/array2d.hpp"
#include "statchem/helper/benchmark.hpp"
#include "statchem/helper/help.hpp"
#include "candock/linker/state.hpp"
#include "statchem/molib/bond.hpp"
#include "statchem/molib/molecule.hpp"
#include "statchem/score/score.hpp"

using namespace std;

namespace candock {
namespace linker {

ostream& operator<<(ostream& os, const Partial& le) {
    os << "start link ++++++++++++++++++++++++++++++" << endl;
    os << "ENERGY = " << le.get_energy() << endl;
    for (auto& pstate : le.get_states())
        //~ os << *pstate << endl;
        os << pstate->pdb() << endl;
    os << "end link --------------------------------" << endl;
    return os;
}

ostream& operator<<(ostream& os, const Partial::Vec& vec_le) {
    for (auto& le : vec_le) {
        os << le << endl;
    }
    return os;
}

//~ double Partial::compute_rmsd_ord(const Partial &other) const {
//~ geometry::Point::Vec crds1, crds2;
//~ map<const molib::Atom*, pair<geometry::Point, geometry::Point>> crd_map;
//~
//~ for (auto &pstate : this->get_states()) {
//~ for (int i = 0; i < pstate->get_segment().get_atoms().size(); ++i) {
//~ auto &a = pstate->get_segment().get_atom(i);
//~ crd_map[&a].first = pstate->get_crd(i);
//~ }
//~ }
//~ for (auto &pstate : other.get_states()) {
//~ for (int i = 0; i < pstate->get_segment().get_atoms().size(); ++i) {
//~ auto &a = pstate->get_segment().get_atom(i);
//~ crd_map[&a].second = pstate->get_crd(i);
//~ }
//~ }
//~ for (auto &kv : crd_map) {
//~ auto &crd1 = kv.second.first;
//~ auto &crd2 = kv.second.second;
//~ if (crd1 == geometry::Point() || crd2 == geometry::Point())
//~ throw Error("die : cannot compute rmsd between two partial conformations,
//which have different compositions");
//~ crds1.push_back(crd1);
//~ crds2.push_back(crd2);
//~ }
//~ return geometry::compute_rmsd(crds1, crds2);
//~ }

double Partial::compute_rmsd_ord(const Partial& other) const {
    double sum_squared = 0;
    int sz = 0;
    set<Segment::Id> segid1;
    for (auto& pstate1 : this->get_states()) {
        segid1.insert(pstate1->get_segment().get_id());
    }
    for (auto& pstate2 : other.get_states()) {
        if (!segid1.count(pstate2->get_segment().get_id()))
            throw Error(
                "die : cannot compute rmsd between two partial conformations "
                "with different compositions");
    }
    for (auto& pstate1 : this->get_states()) {
        for (auto& pstate2 : other.get_states()) {
            if (pstate1->get_segment().get_id() ==
                pstate2->get_segment().get_id()) {
                if (pstate1->get_id() == pstate2->get_id()) {
                    sz += pstate1->get_crds().size();
                } else {
                    auto& crds1 = pstate1->get_crds();
                    auto& crds2 = pstate2->get_crds();
                    for (size_t i = 0; i < crds1.size(); ++i) {
                        sum_squared += crds1[i].distance_sq(crds2[i]);
                    }
                    sz += crds1.size();
                }
            }
        }
    }
    return sqrt(sum_squared / sz);
}

geometry::Point Partial::compute_geometric_center() const {
    return geometry::compute_geometric_center(this->get_ligand_crds());
}

void Partial::sort(Partial::Vec& v) {
    std::sort(v.begin(), v.end(), Partial::comp());
}

geometry::Point::Vec Partial::get_ligand_crds() const {
    geometry::Point::Vec crds;
    for (auto& pstate : __states) {
        for (auto& crd : pstate->get_crds()) {
            crds.push_back(crd);
        }
    }
    return crds;
}

void Partial::set_ligand_crds(const geometry::Point::Vec& crds) {
    int i = 0;
    for (auto& pstate : __states) {
        for (auto& crd : pstate->get_crds()) {
            crd = crds[i++];
        }
    }
}

molib::Atom::Vec Partial::get_ligand_atoms() {
    molib::Atom::Vec atoms;
    for (auto& pstate : __states) {
        for (auto& patom : pstate->get_segment().get_atoms()) {
            atoms.push_back(patom);
        }
    }
    return atoms;
}
}
}
