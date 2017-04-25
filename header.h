#include <stdio.h>
#include <Eigen/Sparse>

using namespace Eigen;

typedef SparseMatrix<double> SparseMat;

typedef std::pair<int,double> SparsePair;
typedef std::vector<SparsePair> SparseVec;
typedef std::function<SparseVec(int)> SparseColFn;

// Convenience function to create a sparse matrix.
static inline SparseMat *sparseCol(SparseColFn f, int n, int m) {
    SparseMat *A = new SparseMat(n,m);
    typedef Triplet<double> T;
    std::vector<T> trip;
    
    for(int col=0; col<m; col++) {
        SparseVec pr = f(col);
        for(std::vector<SparsePair>::iterator it = pr.begin();
                it != pr.end(); it++) {
            trip.push_back(T(it->first, col, it->second));
        }
    }

    A->setFromTriplets(trip.begin(), trip.end());
    return A;
}

extern "C" {

#include <sil_ext.h>
#include <proto_prim.h>

//static const unsigned char sparse_hash[HASH_SIZE+1] = /*!hash!*/;
static const unsigned char sparse_hash[HASH_SIZE+1] = "";

static inline int sil_pushSparse(sil_State *S, SparseMat *M) {
    return sil_newuserdata(S, sparse_hash, M);
}

}
