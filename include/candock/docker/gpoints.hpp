/* This is gpoints.hpp and is part of CANDOCK
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

#ifndef GPOINTS_H
#define GPOINTS_H
#include "candock/centro/centroids.hpp"

#include "statchem/geometry/geometry.hpp"
#include "statchem/helper/array1d.hpp"
#include "statchem/helper/array3d.hpp"
#include "statchem/helper/error.hpp"

#include "statchem/score/score.hpp"

namespace candock {
namespace docker {

using namespace statchem;

class Gpoints {
   public:
    struct IJK {
        int i, j, k;
        IJK operator+(const IJK& right) const {
            return IJK{i + right.i, j + right.j, k + right.k};
        }
        IJK operator-(const IJK& right) const {
            return IJK{i - right.i, j - right.j, k - right.k};
        }
        friend std::ostream& operator<<(std::ostream& os, const IJK& ijk) {
            os << ijk.i << " " << ijk.j << " " << ijk.k << std::endl;
            return os;
        }
    };

    struct Gpoint {
        geometry::Point __crd;
        IJK __ijk;
        Array1d<double> __energy;  // ligand idatm type to energy
        geometry::Point& crd() { return __crd; }
        double energy(const int l) const { return __energy.data[l]; }
        const geometry::Point& crd() const { return __crd; }
        IJK& ijk() { return __ijk; }
        const IJK& ijk() const { return __ijk; }
        void distance(double) const {}  // just dummy : needed by grid
    };

    typedef std::vector<Gpoint> GpointVec;
    typedef std::vector<Gpoint*> PGpointVec;

   private:
    std::map<int, GpointVec> __gridpoints;
    std::map<int, Array3d<Gpoint*>> __gmap;
    const score::Score* __score;
    const std::set<int>* __ligand_idatm_types;

    void __identify_gridpoints(const double& grid_spacing,
                               const double& radial_check);
    void __identify_gridpoints(const centro::Centroids& centroids,
                               const molib::Atom::Grid& grid,
                               const double& grid_spacing,
                               const int& dist_cutoff,
                               const double& excluded_radius,
                               const double& max_interatomic_distance);

   public:
    Gpoints(const double& grid_spacing, const double& radial_check);
    Gpoints(const score::Score& score, const std::set<int>& ligand_idatm_types,
            const centro::Centroids& centroids, const molib::Atom::Grid& grid,
            const double& grid_spacing, const int& dist_cutoff,
            const double& excluded_radius,
            const double& max_interatomic_distance);
    Gpoints(const centro::Centroids& centroids, molib::Atom::Grid& grid,
            const double& grid_spacing, const int& dist_cutoff,
            const double& excluded_radius,
            const double& max_interatomic_distance);
    GpointVec& get_gridpoints0() {
        if (__gridpoints.count(0) != 0) {
            return __gridpoints.at(0);
        }

        throw Error("die : no gridpoints0 ?");
    }
    const GpointVec& get_gridpoints0() const {
        if (__gridpoints.count(0) != 0) {
            return __gridpoints.at(0);
        }

        throw Error("die : no gridpoints0 ?");
    }
    const std::map<int, GpointVec>& get_gridpoints() const {
        return __gridpoints;
    }
    const Gpoint& get_center_point() const;
    const Array3d<Gpoint*>& get_gmap(const int bsite_id) const {
        if (__gmap.count(bsite_id) != 0) {
            return __gmap.at(bsite_id);
        }

        throw Error("die : cannot get gmap for bsite #" +
                    std::to_string(bsite_id));
    }

    friend std::ostream& operator<<(std::ostream& os, const Gpoints& gpoints);
};

std::ostream& operator<<(std::ostream& os,
                         const docker::Gpoints::GpointVec& points);
std::ostream& operator<<(std::ostream& os,
                         const docker::Gpoints::PGpointVec& points);
};
}

#endif
