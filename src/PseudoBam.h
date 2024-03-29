#ifndef KALLISTO_PSEUDOBAM_H
#define KALLISTO_PSEUDOBAM_H

#include "common.h"
#include <vector>
#include <iostream>
#include <utility>

#include "KmerIndex.h"
#include "GeneModel.h"

#ifndef NO_HTSLIB
#include <htslib/sam.h>
#include <htslib/hts.h>
#include <htslib/bgzf.h>
void outputPseudoBam(const KmerIndex &index, const Roaring& u,
                    const char *s1, const char *n1, const char *q1, int slen1, int nlen1, const std::vector<std::pair<KmerEntry,int>>& v1,
                    const char *s2, const char *n2, const char *q2, int slen2, int nlen2, const std::vector<std::pair<KmerEntry,int>>& v2,
                    bool paired, bam_hdr_t *h, samFile *fp);
void revseq(char *b1, char *b2, const char *s, const char *q, int n);
void getCIGARandSoftClip(char* cig, bool strand, bool mapped, int &posread, int &posmate, int length, int targetlength);
bam_hdr_t* createPseudoBamHeaderTrans(const KmerIndex& index);
bam_hdr_t* createPseudoBamHeaderGenome(const Transcriptome& model);
#endif

struct PseudoAlignmentInfo {
  int32_t id; // id local to batch
  bool paired;  // 1
  bool r1empty; // 2
  bool r2empty; // 4
  int k1pos; // -1 for not present, 0-based position of first mapping k-mer in read 1
  int k2pos;
  Roaring ec;
  uint64_t barcode;
  uint64_t UMI;

  PseudoAlignmentInfo() : id(-1), r1empty (true), r2empty(true), paired(true), k1pos(-1), k2pos(-1), barcode(0), UMI(0) {}
};


struct PseudoAlignmentBatch {
  int32_t batch_id;
  std::vector<PseudoAlignmentInfo> aln;
  PseudoAlignmentBatch() : batch_id(-1) {}
};



void writePseudoAlignmentBatch(std::ofstream& of, const PseudoAlignmentBatch& batch);
void readPseudoAlignmentBatch(std::ifstream& in, PseudoAlignmentBatch& batch);


#endif // KALLISTO_PSEUDOBAM_H
