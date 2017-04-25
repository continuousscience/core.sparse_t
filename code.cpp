#include "header.h"
extern "C" {
char *show(const SparseMat *A) {
    int m = A->nonZeros();
    char *out;

    if(m == 0) {
        asprintf(&out, "sparse(%d, %d)", A->rows(), A->cols());
    } else {
        int i = 0, len = 0;
        if(m > 10) {
            m = 10;
            out = (char *)malloc(m*25 + 6);
        } else {
            out = (char *)malloc(m*25 + 2);
        }
        for(int j=0; j<A->outerSize(); ++j) {
          for(SparseMat::InnerIterator it(*A,j); it; ++it) {
            if(len == 0) {
                len = snprintf(out, 25, "[ (%d, %d, %g)",
                                        it.row(), j, it.value());
            } else {
                len += snprintf(out+len, 25, ", (%d, %d, %g)",
                                        it.row(), j, it.value());
            }
            if(++i >= 10) {
                len += snprintf(out+len, 3, "...");
                break;
            }
          }
        }
        snprintf(out+len, 2, "]");
    }
    return out;
}

size_t size(const SparseMat *A) {
    size_t len = size_uint32(A->rows()) + size_uint32(A->cols());

    for(int j=0; j<A->outerSize(); ++j) {
        len += size_int32(-j);
        for(SparseMat::InnerIterator it(*A,j); it; ++it) {
            len += size_int32(it.row());
            len += 8;
        }
    }
    return len;
}

void serialize(SWriter *s, const SparseMat *A) {
    write_uint32(s, A->rows());
    write_uint32(s, A->cols());

    for(int j=0; j<A->outerSize(); ++j) {
        write_int32(s, -j);
        for(SparseMat::InnerIterator it(*A,j); it; ++it) {
            write_int32(s, it.row());
            write_fixed64(s, it.value());
        }
    }
}

void parse(sil_State *S, const uint8_t *buf, size_t len) {
    uint32_t n, m;
    int i, j = 0;
    double x;
    unsigned k = read_uint32(&n, buf, len);
    buf += k; len -= k;
    k = read_uint32(&m, buf, len);
    buf += k; len -= k;

    SparseMat *M = new SparseMat(n,m);
    typedef Triplet<double> T;
    std::vector<T> trip;

    while(len > 0) {
        k = read_int32(&i, buf, len);
        buf += k; len -= k;
        if(i < 0) { // new col
            j = -i;
            continue;
        }
        k = read_fixed64(&x, buf, len);
        buf += k; len -= k;
        trip.push_back(T(i, j, x));
    }

    M->setFromTriplets(trip.begin(), trip.end());
    sil_pushSparse(S, M);
    return;
}

void handler(SparseMat *M) {
    delete M;
}

void copy(sil_State *S) {
    size_t len; // Always assume len is wrong!
    SparseMat *M = (SparseMat *)sil_getST(S, &len);
    if(M == NULL) {
        sil_err(S, "Can't copy - no matrix present.");
        return;
    }
    SparseMat *C = new SparseMat(*M);
    sil_setST(S, C, len);
}
}
