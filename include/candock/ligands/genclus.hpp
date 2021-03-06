/* This is genclus.hpp and is part of CANDOCK
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

#ifndef GENCLUS_HPP
#define GENCLUS_HPP

#include <string>

namespace candock {

namespace genclus {
void generate_clusters_of_ligands(
    const std::string& json_file, const std::string& json_with_ligs_file,
    const std::string& bio_dir, const std::string& names_dir, const bool neighb,
    const double hetero_clus_rad, const int hetero_min_pts,
    const double min_z_score, const bool for_gclus = false);
}
}

#endif
