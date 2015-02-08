#include "cluster.h"
#include "product.h"
#include "subgraph.h"
#include "item.h"
#include "debug.hpp"

void Cluster::cluster(Subgraph *s) {

#ifndef NDEBUG
  clock_t start = clock();
#endif

  /* copy all qmax-es to the clus vector */
  dbgmsg("CLUS> Cluster "<<s->qmax.size()<<" maximum cliques.");
  for (unsigned int j = 0; j < s->qmax.size(); j++) {
    s->clus.push_back(new Item() );
    s->clus.back()->vert.reserve(s->qmax[j]->vert.size() );
    s->clus.back()->copy_unique(s->qmax[j]); // doesn't need to be copy_unique but doesn't hurt also :)
    
    
    gsl_matrix_memcpy(s->clus.back()->U, s->qmax[j]->U);
    gsl_vector_memcpy(s->clus.back()->t, s->qmax[j]->t);
    
    s->clus.back()->calpha_rmsd             = s->qmax[j]->calpha_rmsd;
    s->clus.back()->score_descriptor_ratio  = s->qmax[j]->score_descriptor_ratio;
    s->clus.back()->surf_vector_angle       = s->qmax[j]->surf_vector_angle;
    s->clus.back()->score_probe             = s->qmax[j]->score_probe;
    
    
  }
  
  unsigned int cs = 0;

  for (unsigned int i= 0; i < s->clus.size(); i++) {
    s->clus[i]->N = 1;
  }

  /* klastriraj, dokler se stevilo klastrov ne ustali */
  while ( s->clus.size() != cs ) {
#ifndef NDEBUG
    dbgmsg("Cluster::cluster new iteration s->clus.size() = "<<s->clus.size());
#endif    
    cs = s->clus.size();
    for (unsigned int i = 0; i < s->clus.size(); i++) {
      for (unsigned int j = i + 1; j < s->clus.size(); j++) {

        if ( num_common(s->clus[i], s->clus[j]) > CLUS_SPEC ) {
#ifndef NDEBUG
          dbgmsg("Cluster::cluster joining "<<i<<"-th and "<<j<<"-th cluster");
#endif
          s->clus[i]->copy_unique(s->clus[j]); // ce sta za skupaj, dodaj i-ju j-ti qmax

          /* po novem ima cluster ROTA in trans matrix kar od najvecje klike (ne povprecimo, ker pride do napak) */
          s->clus[i]->calpha_rmsd            += s->clus[j]->calpha_rmsd;                  
          s->clus[i]->score_descriptor_ratio += s->clus[j]->score_descriptor_ratio;
          s->clus[i]->surf_vector_angle      += s->clus[j]->surf_vector_angle;  
          s->clus[i]->score_probe            += s->clus[j]->score_probe;           

          s->clus[i]->N += s->clus[j]->N;

          delete s->clus[j];
          s->clus.erase(s->clus.begin() + j);   // izbrisemo klaster j
        }
      }

      
    }
  }

  /* izracunamo vse povprecne score za i-ti cluster in njihove standardne deviacije */
  for (unsigned int i = 0; i < s->clus.size(); i++) {

    /* v i-ti klaster zapisemo povprecne vrednosti scoreov in matrixov */
    s->clus[i]->calpha_rmsd             = s->clus[i]->calpha_rmsd               / s->clus[i]->N;
    s->clus[i]->score_descriptor_ratio  = s->clus[i]->score_descriptor_ratio    / s->clus[i]->N;
    s->clus[i]->surf_vector_angle       = s->clus[i]->surf_vector_angle         / s->clus[i]->N;
    s->clus[i]->score_probe             = s->clus[i]->score_probe               / s->clus[i]->N;
    // do not copy score_blosum or cluster_score, because they are calculated later for each cluster


#ifndef NDEBUG
    dbgmsg("Cluster::cluster s->clus["<<i<<"]->N="<<s->clus[i]->N);
#endif

  }

  dbgmsg("CLUS> Done clustering. Left with "<<s->clus.size()<<" clusters.");

  dbgmsg("Cluster::cluster() Time = " << (double) (clock() - start) / CLOCKS_PER_SEC);

}


int Cluster::num_common(Item *clusI, Item *clusJ) {
  int st = 0;
  for (unsigned int i = 0; i < clusI->vert.size(); i++)
    for (unsigned int j = 0; j < clusJ->vert.size(); j++) {
      if ((clusI->vert[i]->row->desc1 == clusJ->vert[j]->row->desc1) && (clusI->vert[i]->row->desc2 == clusJ->vert[j]->row->desc2)) {
        st++;
        break;
      }
    }
  return st;
}

