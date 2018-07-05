#ifndef DOCK_H
#define DOCK_H

#include "candock/docker/gpoints.hpp"
#include "candock/docker/conformations.hpp"
#include "candock/molib/molecules.hpp"

namespace Molib {
	class Molecule;
};

namespace Docker {
	
	class Dock {

		class DockedConf {
		public:
			typedef vector<DockedConf> Vec;

		private:
			const Gpoints::Gpoint &__cavpoint;
			Gpoints::PGpointVec &__conf0;
			double __energy;
			size_t __i;
			size_t __bsite_id;
		public:
			DockedConf(const Gpoints::Gpoint &cavpoint, Gpoints::PGpointVec &conf0, double energy, size_t i, size_t bsite_id) 
				: __cavpoint(cavpoint), __conf0(conf0), __energy(energy), __i(i), __bsite_id(bsite_id) {}
			//geometry::Point& crd() { return __cavpoint.crd(); }
			const geometry::Point& crd() const { return __cavpoint.crd(); }
			void distance(const double) const {} // dummy
			const Gpoints::Gpoint &get_cavpoint() const { return __cavpoint; }
			const Gpoints::PGpointVec &get_conf0() const { return __conf0; }
			double get_energy() const { return __energy; }
			size_t get_i() const { return __i; }
			size_t get_bsite_id() const { return __bsite_id; }
			struct by_energy {
				bool operator() (const DockedConf *lhs, const DockedConf *rhs) const {
					return lhs->__energy < rhs->__energy;
				}
			};			
			double compute_rmsd_sq(const DockedConf &other) const;
		};

		const Gpoints &__gpoints;
		Conformations &__conformations;
		const Molib::Molecule &__seed;
		double __rmsd_tol_sq;
                double __rmsd_tol;

		Molib::Molecules __docked;

#ifndef NDEBUG
		const Score::Score &__score;
		const Molib::Atom::Grid &__gridrec;
#endif
		DockedConf::Vec __dock();
		DockedConf::Vec __cluster(const DockedConf::Vec &confs);
		void __cluster_fast(const DockedConf::Vec &conformations, DockedConf::Vec &reps);
		void __set_docked(const DockedConf::Vec &confs);
		
	public:
#ifndef NDEBUG
		Dock(const Gpoints &gpoints, Conformations &conformations, const Molib::Molecule &seed, const Score::Score &score, 
			 const Molib::Atom::Grid &gridrec, const double rmsd_tol=2.0) : __gpoints(gpoints), __conformations(conformations), 
			__seed(seed), __rmsd_tol_sq(rmsd_tol * rmsd_tol), __rmsd_tol(rmsd_tol), __score(score), __gridrec(gridrec) {}
#else
		Dock(const Gpoints &gpoints, Conformations &conformations, Molib::Molecule &seed, const double rmsd_tol=2.0) 
			: __gpoints(gpoints), __conformations(conformations), __seed(seed), __rmsd_tol_sq(rmsd_tol * rmsd_tol), __rmsd_tol(rmsd_tol) {}
#endif
		void run();
		Molib::Molecules& get_docked() { return __docked; };
	};
};

#endif