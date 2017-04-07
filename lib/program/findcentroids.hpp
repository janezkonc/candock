#ifndef FINDCENTROIDS_H
#define FINDCENTROIDS_H

#include "programstep.hpp"
#include "centro/centroids.hpp"
#include "molib/molecule.hpp"

namespace Program {

	class FindCentroids : public ProgramStep
	{
	protected:
		virtual bool __can_read_from_files();
		virtual void __read_from_files();
		virtual void __continue_from_prev();
		
		const Molib::Molecule& __receptor;
                const std::string&     __filename;

		Centro::Centroids __result;
                std::string __centroid_file;

	public:
		FindCentroids ( const Molib::Molecule& receptor, const std::string& filename );
                virtual ~FindCentroids() {}

		const Centro::Centroids& centroids() const {
			return __result;
		}

	};

}

#endif
