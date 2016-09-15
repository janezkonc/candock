#ifndef FRAGMENTLIGANDS_H
#define FRAGMENTLIGANDS_H

#include <set>
#include <mutex>

#include "programstep.hpp"
#include "pdbreader/pdbreader.hpp"
#include "pdbreader/molecules.hpp"

namespace Program {

	class FragmentLigands : public ProgramStep
	{
		Molib::Molecules __seeds;
		std::set<int> __ligand_idatm_types;
		std::set<int> __added;

		//Not Used until to ability to avoid rereading from disk is enabled
		//Molib::Molecules __ligands;

		std::mutex __prevent_re_read_mtx;
		std::mutex __add_to_typing_mtx;

		void __fragment_ligands ( Molib::PDBreader& lpdb, const CmdLnOpts& cmdl );

	protected:
		virtual bool __can_read_from_files(const CmdLnOpts& cmdl);
		virtual void __read_from_files(const CmdLnOpts& cmdl);
		virtual void __continue_from_prev(const CmdLnOpts& cmdl);

	public:
		FragmentLigands( ) { }

		const Molib::Molecules& seeds() const {
			return __seeds;
		}

		const std::set<int>& ligand_idatm_types() const {
			return __ligand_idatm_types;
		}

	};

}

#endif
