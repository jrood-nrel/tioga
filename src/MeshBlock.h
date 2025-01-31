//
// This file is part of the Tioga software library
//
// Tioga  is a tool for overset grid assembly on parallel distributed systems
// Copyright (C) 2015 Jay Sitaraman
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA

#ifndef MESHBLOCK_H
#define MESHBLOCK_H

#include "ADT.h"
#include "TiogaMeshInfo.h"
#include "codetypes.h"
#include <algorithm>
#include <assert.h>
#include <stdint.h>
#include <vector>

// forward declare to instantiate one of the methods
class parallelComm;
class CartGrid;

/**
 * MeshBlock class - container and functions for generic unstructured grid
 * partition in 3D
 *
 * Jay Sitaraman
 * 02/20/2014
 */

#define FRINGE (-1)
#define HOLE 0
#define FIELD 1

class MeshBlock
{
private:
    /** Mesh block info provided by the application code
     *
     *  This is a non-owning pointer whose lifetime is controlled by the
     *  application.
     */
    TIOGA::MeshBlockInfo* m_info{nullptr};

    /** Device copy of the mesh block info registered by the application code
     *
     *  This pointer is owned by MeshBlock
     */
    TIOGA::MeshBlockInfo* m_info_device{nullptr};

    int nnodes; /** < number of grid nodes */
    int ncells; /** < total number of cells */
    int ntypes; /** < number of different types of cells */
    int* nv;    /** < number of vertices for each types of cell */
    int* nc;    /** < number of each of different kinds of cells (tets, prism,
                   pyramids, hex etc) */
    int nobc;   /** < number of overset boundary nodes */
    int nwbc;   /** < number of wall boundary nodes */
    int nvar;   /** < Number of variables */
    //
    double* x;        /** < grid nodes x[3*nnodes] */
    int* iblank_cell; /** < iblank value at each grid cell */
    //
    int** vconn;       /** < connectivity of each kind of cell */
    int* wbcnode;      /** < wall boundary node indices */
    int* obcnode;      /** < overset boundary node indices */
    uint64_t* cellGID; /** < Global ID of the cell */
    uint64_t* nodeGID; /** < Global ID for the nodes */
    //
    int nwbcface;                  /** < number of wall boundary faces */
    int nobcface;                  /** < number of outer boundary faces */
    std::vector<int> wbcfacenode;  /** < [4*nwbcface] wall face boundary nodes
                                      (-1  node: triangle face) */
    std::vector<int> obcfacenode;  /** < [4*nobcface] outer face boundary nodes
                                      (-1  node: triangle face) */
    std::vector<box_t> wbcfacebox; /** < [nwbcface] wall face bounding boxes */
    std::vector<box_t> obcfacebox; /** < [nobcface] outer face bounding boxes */
    //
    double* nodeRes; /** < node resolution  */
    double* userSpecifiedNodeRes;
    double* userSpecifiedCellRes;
    double* elementBbox; /** < bounding box of the elements */
    int* elementList;    /** < list of elements in */
    //
    // Alternating digital tree library
    //
    ADT* adt; /** < Digital tree for searching this block */
    //
    DONORLIST** donorList; /**< list of donors for the nodes of this mesh */
    //
    int ninterp; /**< number of interpolations to be performed */
    int interpListSize;
    int interptype;
    INTERPLIST* interpList; /**< list of donor nodes in my grid, with fractions
                               and information of who they donate to */
    int* interp2donor;

    INTEGERLIST*
        cancelList; /** receptors that need to be canceled because of */
    int ncancel;    /** conflicts with the state of their donors */
    void (*get_nodes_per_cell)(int*, int*);
    void (*get_receptor_nodes)(int*, int*, double*);
    void (*donor_inclusion_test)(int*, double*, int*, double*);
    void (*donor_frac)(int*, double*, int*, int*, double*, double*, int*);
    void (*convert_to_modal)(int*, int*, double*, int*, int*, double*);

    int nreceptorCells;   /** number of receptor cells */
    int* ctag;            /** index of receptor cells */
    int* pointsPerCell;   /** number of receptor points per cell */
    int maxPointsPerCell; /** max of pointsPerCell vector */
    double* rxyz;         /** point coordinates */
    int ipoint;
    int* picked; /** < flag specifying if a node has been selected for
                    high-order interpolation */

    int nreceptorCellsCart;
    int* ctag_cart;
    int* pickedCart;
    int uniform_hex;
    double dx[3];
    double xlow[3];
    int idims[3];
    int* uindx;
    int* invmap;     // inverse map
    int* mapmask;    // mask
    int* icft;       // frequency table for nodal containment
    int mapdims[3];  // dimensions of the map
    double mapdx[3]; // sides of the map
public:
    int* iblank; /** < iblank value for each grid node */
    int* iblank_reduced;
    int ntotalPointsCart;
    double* rxyzCart;
    int* donorIdCart;
    int donorListLength;

    int nfringe;
    int mexclude;
    int meshtag; /** < tag of the mesh that this block belongs to */
    int check_uniform_hex_flag;
    double resolutionScale;
    double searchTol;
    int dominanceFlag; /**< if dominanceflag=1: set noderes to tiny number */
    //
    // oriented bounding box of this partition
    //
    OBB* obb;
    OBB* obh;
    //
    int nsearch;        /** < number of query points to search in this block */
    int* isearch;       /** < index of query points in the remote process */
    int* tagsearch;     /** < index of query points in the remote process */
    double* res_search; /** < resolution of search points */
    int* xtag;          /** < hash to determine if there are duplicates */
    double* xsearch;    /** < coordinates of the query points */
    double* rst;        /** < natural coordinates */
    int* donorId;       /** < donor indices for those found */
    std::vector<uint64_t>
        gid_search; /** < Global node ID for the query points */
    int donorCount;

    int myid;               /** < global mpi rank */
    int blockcomm_id;       /** < mpi rank within this block */
    int blockcomm_numprocs; /** < number of mpi ranks within this blocks */
    MPI_Comm blockcomm;     /** < subgroup communicator for this mesh block */
    char composite;         /** < mesh block composite body flag */
    char meshMaster; /** < master rank for each body flag: [0] subordinate rank,
                        [1] master rank */
    double bboxLocalAHM[6]; /** < wall bc node bounding box (without duplicate
                               obc/wbc nodes) */

    double* cellRes;  /** < resolution for each cell */
    int ntotalPoints; /**  total number of extra points to interpolate */
    int ihigh;
    int ninterp2; /** < number of interpolants for high-order points */
    int interp2ListSize;
    INTERPLIST* interpList2; /** < list for high-interpolation points */
    int ninterpCart;
    int interpListCartSize;
    INTERPLIST* interpListCart;
    int* receptorIdCart;

    int* vconn_ptrs[TIOGA::MeshBlockInfo::max_vertex_types];

    //
    // call back functions to use p4est to search
    // its own internal data
    //
    void (*p4estsearchpt)(double*, int*, int*, int*);
    void (*check_intersect_p4est)(int*, int*);

    /** basic constructor */
    MeshBlock()
    {
        nv = nullptr;
        nc = nullptr;
        x = nullptr;
        iblank = nullptr;
        iblank_cell = nullptr;
        vconn = nullptr;
        wbcnode = nullptr;
        obcnode = nullptr;
        cellRes = nullptr;
        nodeRes = nullptr;
        elementBbox = nullptr;
        elementList = nullptr;
        adt = nullptr;
        donorList = nullptr;
        interpList = nullptr;
        interp2donor = nullptr;
        obb = nullptr;
        nsearch = 0;
        isearch = nullptr;
        tagsearch = nullptr;
        res_search = nullptr;
        xsearch = nullptr;
        donorId = nullptr;
        xtag = nullptr;
        adt = nullptr;
        cancelList = nullptr;
        userSpecifiedNodeRes = nullptr;
        userSpecifiedCellRes = nullptr;
        nfringe = 1;
        composite = 0;
        mexclude = 3;
        nvar = 0;
        interptype = 0;
        blockcomm = MPI_COMM_NULL;
        // new vars
        ninterp = ninterp2 = interpListSize = interp2ListSize = 0;
        ctag = nullptr;
        pointsPerCell = nullptr;
        maxPointsPerCell = 0;
        rxyz = nullptr;
        ntotalPoints = 0;
        rst = nullptr;
        ihigh = 0;
        ipoint = 0;
        interpList2 = nullptr;
        picked = nullptr;
        ctag_cart = nullptr;
        rxyzCart = nullptr;
        donorIdCart = nullptr;
        pickedCart = nullptr;
        ntotalPointsCart = 0;
        nreceptorCellsCart = 0;
        ninterpCart = 0;
        interpListCartSize = 0;
        interpListCart = nullptr;
        resolutionScale = 1.0;
        receptorIdCart = nullptr;
        searchTol = TOL;
        dominanceFlag = 0;

        cellGID = nullptr;
        iblank_reduced = nullptr;
        uniform_hex = 0;
        check_uniform_hex_flag = 0;
        uindx = nullptr;
        obh = nullptr;
        invmap = nullptr;
        icft = nullptr;
        mapmask = nullptr;
    };

    /** basic destructor */
    ~MeshBlock();

    void preprocess(int use_adaptholemap);

    void tagBoundary();

    void tagBoundaryFaces();

    void writeGridFile(int bid);

    void writeFlowFile(int bid, double* q, int nvar, int type);

    void setData(TIOGA::MeshBlockInfo* minfo);

    void setData(
        int btag,
        int nnodesi,
        double* xyzi,
        int* ibli,
        int nwbci,
        int nobci,
        int* wbcnodei,
        int* obcnodei,
        int ntypesi,
        int* nvi,
        int* nci,
        int** vconni,
        uint64_t* cell_gid = nullptr,
        uint64_t* node_gid = nullptr);

    void setResolutions(double* nres, double* cres);

    void search();
    void search_uniform_hex();
    void writeOBB(int bid) const;

    void writeOBB2(OBB* obc, int bid);

    void updateSolnData(int inode, const double* qvar, double* q) const;

    int getNinterp() const { return ninterp; };

    void getInterpolatedSolution(
        int* nints,
        int* nreals,
        int** intData,
        double** realData,
        const double* q,
        int nvar,
        int interptype);

    void getInterpolatedSolutionAMR(
        int* nints,
        int* nreals,
        int** intData,
        double** realData,
        const double* q,
        const int* /*sndMap*/);

    void checkContainment(int* cellIndex, int adtElement, double* xsearch);

    void getWallBounds(int* mtag, int* existWall, double wbox[6]);

    void markWallBoundary(int* sam, int nx[3], const double extents[6]);

    void markBoundaryAdaptiveMap(
        char nodetype2tag,
        const double extents_lo[3],
        const double extents_hi[3],
        level_octant_t* level,
        const uint8_t* taggedList,
        uint8_t* tagList);

    void markBoundaryAdaptiveMapSurfaceIntersect(
        char nodetype2tag,
        const double extents_lo[3],
        const double extents_hi[3],
        level_octant_t* level,
        const uint8_t* taggedList,
        uint8_t* tagList);

    void markBoundaryAdaptiveMapSurfaceIntersect(
        char nodetype2tag,
        const double extents_lo[3],
        const double extents_hi[3],
        uint8_t level_id,
        uint32_t noctants,
        octant_coordinates_t* octants,
        const uint8_t* taggedList,
        uint8_t* tagList);

    void markBoundaryMapSurface(
        char nodetype2tag,
        double extents_lo[3],
        double extents_hi[3],
        level_octant_t* level,
        uint8_t* taggedList,
        uint8_t* tagList);

    void getQueryPoints(
        OBB* obc, int* nints, int** intData, int* nreals, double** realData);

    void getQueryPoints2(
        OBB* obc, int* nints, int** intData, int* nreals, double** realData);

    /** routines that do book keeping */

    void getDonorPacket(PACKET* sndPack, int nsend) const;

    void initializeDonorList();

    void insertAndSort(
        int pointid,
        int senderid,
        int meshtag,
        int remoteid,
        double donorRes,
        double receptorRes);

    void processDonors(
        HOLEMAP* holemap,
        int nmesh,
        int** donorRecords,
        double** receptorResolution,
        int* nrecords);

    void processDonors(
        ADAPTIVE_HOLEMAP* holemap,
        int nmesh,
        int** donorRecords,
        double** receptorResolution,
        int* nrecords);

    void initializeInterpList(int ninterp_input);

    void findInterpData(int* recid, int irecord, double receptorRes);

    void findInterpListCart();

    void set_ninterp(int /*ninterp_input*/);

    void getCancellationData(int* nints, int** intData);

    void cancelDonor(int irecord);

    void getInterpData(int* nrecords, int** intData);

    void clearIblanks();

    void getStats(int mstat[2]) const;

    void setIblanks(int inode) const;

    void getDonorCount(int* dcount, int* fcount);

    void getDonorInfo(int* receptors, int* indices, double* frac);

    void getReceptorInfo(int* receptors);

    void getReducedOBB(OBB* /*obc*/, double* /*realData*/);
    void getReducedOBB2(OBB* /*obc*/, double* /*realData*/);

    void resetCoincident();
    //
    // routines for high order connectivity and interpolation
    //
    void getCellIblanks();
    void getCellIblanks2();
    void set_cell_iblank(int* iblank_cell_input)
    {
        iblank_cell = iblank_cell_input;
    }
    void setcallback(
        void (*f1)(int*, int*),
        void (*f2)(int*, int*, double*),
        void (*f3)(int*, double*, int*, double*),
        void (*f4)(int*, double*, int*, int*, double*, double*, int*),
        void (*f5)(int*, int*, double*, int*, int*, double*))
    {
        get_nodes_per_cell = f1;
        get_receptor_nodes = f2;
        donor_inclusion_test = f3;
        donor_frac = f4;
        convert_to_modal = f5;
    }

    void setp4estcallback(
        void (*f1)(double*, int*, int*, int*), void (*f2)(int*, int*))
    {
        p4estsearchpt = f1;
        check_intersect_p4est = f2;
    }

    void setCompositeFlag(double tol, int dominanceflag)
    {
        composite = 1;
        searchTol = tol;
        dominanceFlag = dominanceflag;
    }

    void writeCellFile(int /*bid*/);
    void writeBCnodes(char nodetype2tag, int bodyid);
    void getInternalNodes();
    void getExtraQueryPoints(
        OBB* obb, int* nints, int** intData, int* nreals, double** realData);
    void processPointDonors();
    void getInterpolatedSolutionAtPoints(
        int* nints,
        int* nreals,
        int** intData,
        double** realData,
        const double* q,
        int nvar,
        int interptype) const;
    void updatePointData(double* q, double* qtmp, int nvar, int interptype);
    void outputOrphan(FILE* fp, int i)
    {
        fprintf(
            fp, "%f %f %f\n", rxyz[static_cast<int>(3 * i)], rxyz[3 * i + 1],
            rxyz[3 * i + 2]);
    }
    void clearOrphans(HOLEMAP* holemap, int nmesh, const int* itmp);
    void clearOrphans(ADAPTIVE_HOLEMAP* holemap, int nmesh, int* itmp);
    void getUnresolvedMandatoryReceptors();
    void getCartReceptors(CartGrid* cg, parallelComm* pc);
    void fillReceptorDataPtr(
        CartGrid* cg,
        int cell_count,
        int c,
        int j,
        int k,
        int l,
        int* pmap,
        double vol,
        double* xtm,
        bool isNodal,
        INTEGERLIST2*& dataPtr);
    void setCartIblanks();

    // Getters
    inline int getMeshTag() const { return meshtag + (1 - BASE); }
    inline int getWallFlag() const { return static_cast<int>(nwbc > 0); }

    /**
     * Get donor packet for multi-block/partition setups
     *
     */
    void getMBDonorPktSizes(
        std::vector<int>& /*nints*/, std::vector<int>& /*nreals*/) const;

    void getMBDonorPackets(
        std::vector<int>& /*ixOffset*/,
        std::vector<int>& /*rxOffset*/,
        PACKET* /*sndPack*/) const;

    /** Reset interpolation list data structure
     *
     *  Reset the data structures in situations where the performConnectivity
     *  method is invoked at every timestep when meshes undergo relative motion.
     */
    void resetInterpData()
    {
        if (interpList != nullptr) {
            for (int i = 0; i < interpListSize; i++) {
                if (interpList[i].inode != nullptr)
                    TIOGA_FREE(interpList[i].inode);
                if (interpList[i].weights != nullptr)
                    TIOGA_FREE(interpList[i].weights);
            }
            TIOGA_FREE(interpList);
        }
        ninterp = 0;
        interpListSize = 0;
    }
    void reduce_fringes();

    void check_for_uniform_hex();

    void create_hex_cell_map();

    int num_var() const { return nvar; }
    int& num_var() { return nvar; }

    const TIOGA::MeshBlockInfo* mesh_info() const { return m_info; }
    TIOGA::MeshBlockInfo* mesh_info() { return m_info; }

    const TIOGA::MeshBlockInfo* d_mesh_info() const { return m_info_device; }
    TIOGA::MeshBlockInfo* d_mesh_info() { return m_info_device; }

    void set_interptype(int type) { interptype = type; }
    void checkOrphans();

    static char overlapping1D(bound_t box1, bound_t box2)
    {
        return static_cast<char>((box1.hi >= box2.lo) && (box2.hi >= box1.lo));
    }
};

/* Mesh Block Complement Rank Data */
// class meshblockCompInfo {
//   public:
//     int nreq;
//     int id;
//     int nrank;
//     int masterID;   /* master rank for distributing mesh block data */
//     MPI_Comm comm;  /* communicator containing all complement ranks + master
//     */

/* constructor */
//    meshblockCompInfo(){
//        comm = MPI_COMM_NULL;
//    };

/* deconstructor */
//   ~meshblockCompInfo(){
//	int sflag;
//	MPI_Finalized(&sflag);
//	if(!sflag){
//           if(comm != MPI_COMM_NULL) MPI_Comm_free(&comm);
//	}
//    };
//};

#endif /* MESHBLOCK_H */
