#include "candock/program/fragmentligands.hpp"
#include "version.hpp"

using namespace std;

int main(int argc, char* argv[]) {
        try {
#ifndef _WINDOWS
                if ( argc <= 2 ) {
                        cerr << "You MUST supply at least two arguments!" << endl;
                        cerr << "A third argument gives the number of threads (defaults to 1)" << endl;
                        cerr << "A fourth causes top_seeds rmsd calculations" << endl;
                        return 1;
                }

                Inout::Logger::set_all_stderr(true);

                Parser::FileParser crys(argv[1], Parser::pdb_read_options::first_model);
                Molib::Molecules cryst;
                crys.parse_molecule(cryst);

                Molib::Molecules mols1;
                if ( argc >= 5 ) {
                        Parser::FileParser mol1(argv[2], Parser::pdb_read_options::all_models);
                        mol1.parse_molecule(mols1);
                } else {
                        Parser::FileParser mol1(argv[2], Parser::pdb_read_options::docked_poses_only |
                                                         Parser::pdb_read_options::skip_atom |
                                                         Parser::pdb_read_options::all_models);
                        mol1.parse_molecule(mols1);
                }

                if (cryst.size() != 1 ) {
                        cerr << "The crystal ligand file must only have 1 molecule in it" << endl;
                        return 1;
                }

                if (mols1.size() <= 0) {
                        return 0;
                }

                size_t num_threads = 1;
                if (argc >=4) {
                        num_threads = std::atoi(argv[3]);
                }

                cryst[0].erase_properties();
                Molib::Atom::Graph cryst_graph =  Molib::Atom::create_graph(cryst[0].get_atoms());
                std::vector< Molib::Atom::Graph > atom_graphs;

                std::vector<double> rmsd_graph (mols1.size());
                std::vector<double> rmsd_ords  (mols1.size());
                std::vector<double> rmsd_vinas (mols1.size());

                for ( Molib::Molecule& mol : mols1 ) {
                        Molib::Atom::Vec atoms = mol.get_atoms();
                        
                        int reenum = 0;
                        for ( auto &atom : atoms ) {
                                atom->set_atom_number(++reenum);
                                atom->erase_properties();
                        }

                        atom_graphs.push_back( Molib::Atom::create_graph(atoms));
                }

                std::vector<std::thread> threads;

                Molib::Atom::Vec cryst_atoms = cryst[0].get_atoms();

                for ( size_t thread_id = 0; thread_id < num_threads; ++thread_id)
                threads.push_back (std::thread ([&, thread_id] {
                        for (size_t i = thread_id; i < mols1.size(); i+=num_threads) {
                                rmsd_ords[i] = mols1[i].compute_rmsd_ord(cryst[0]);
                                rmsd_graph[i] = Molib::Atom::compute_rmsd(cryst_graph, atom_graphs[i]);

                                Molib::Atom::Vec curr_atoms = mols1[i].get_atoms();
                                rmsd_vinas[i] = std::max(Molib::compute_rmsd_vina(cryst_atoms, curr_atoms),
                                                         Molib::compute_rmsd_vina(curr_atoms, cryst_atoms));
                        }
                }));

                for (auto &thread : threads) {
                        thread.join();
                }

                for (size_t i = 0; i < rmsd_graph.size(); ++i) {
                        std::cout << rmsd_graph[i] << " " << rmsd_ords[i] << " " << rmsd_vinas[i] << '\n';
                }
#endif
        } catch (exception& e) {
                cerr << e.what() << endl;
                return 1;
        }
        return 0;
}