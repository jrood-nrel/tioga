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

#ifndef TIOGA_UTILS_H
#define TIOGA_UTILS_H

/* header files */
#include "tioga.h"

/* system header files */
#include <iostream>
#include <unordered_set>
#include <vector>

struct Node
{
    int id;
    double x, y, z;
    const double eps = 1E-10;

    Node() = default;
    Node(int id, const double* geo)
    {
        this->id = id;
        this->x = geo[0];
        this->y = geo[1];
        this->z = geo[2];
    }

    bool operator==(const Node& otherNode) const
    {
        return (otherNode.id == id) ||
               ((abs(this->x - otherNode.x) <= this->eps) &&
                (abs(this->y - otherNode.y) <= this->eps) &&
                (abs(this->z - otherNode.z) <= this->eps));
    }

    struct HashFunction
    {
        size_t operator()(const Node& node) const
        {
            size_t const xHash = std::hash<int>()(int(node.x));
            size_t const yHash = std::hash<int>()(int(node.y)) << 1;
            size_t const zHash = std::hash<int>()(int(node.z)) << 2;
            return xHash ^ yHash ^ zHash;
        }
    };
};

/* function declarations */
void findOBB(
    double* x, double xc[3], double dxc[3], double vec[3][3], int nnodes);
int checkHoleMap(
    const double* x, const int* nx, int* sam, const double* extents);
int checkAdaptiveHoleMap(double* xpt, ADAPTIVE_HOLEMAP* AHM);
void fillHoleMap(int* holeMap, const int ix[3], int isym);
void octant_children(
    uint8_t children_level,
    uint32_t idx,
    octant_full_t* q,
    octant_full_t* c0,
    octant_full_t* c1,
    octant_full_t* c2,
    octant_full_t* c3,
    octant_full_t* c4,
    octant_full_t* c5,
    octant_full_t* c6,
    octant_full_t* c7);
void octant_children_neighbors(
    const octant_full_t* q,
    octant_full_t* c0,
    octant_full_t* c1,
    octant_full_t* c2,
    octant_full_t* c3,
    octant_full_t* c4,
    octant_full_t* c5,
    octant_full_t* c6,
    octant_full_t* c7);
void floodfill_level(level_octant_t* level);
int obbIntersectCheck(
    double vA[3][3],
    const double xA[3],
    const double dxA[3],
    double vB[3][3],
    const double xB[3],
    const double dxB[3]);
void getobbcoords(
    const double xc[3], const double dxc[3], double vec[3][3], double xv[8][3]);
void transform2OBB(
    const double xv[3], const double xc[3], double vec[3][3], double xd[3]);
void writebbox(OBB* obb, int bid);
void writebboxdiv(OBB* obb, int bid);
void writePoints(double* x, int nsearch, int bid);
void uniquenodes(
    double* x, const int* meshtag, double* rtag, int* itag, const int* nn);
void uniqNodesTree(
    double* coord,
    int* itag,
    double* rtag,
    int* meshtag,
    int* elementsAvailable,
    int ndim,
    int nav);
void uniquenodes_octree(
    double* x, int* meshtag, double* rtag, int* itag, const int* nn);

void qcoord_to_vertex(
    qcoord_t x, qcoord_t y, qcoord_t z, const double* vertices, double vxyz[3]);
char checkFaceBoundaryNodes(
    const int* nodes,
    const char* bcnodeflag,
    const int numfaceverts,
    const int* faceConn,
    const char* duplicatenodeflag);
int triBoxOverlap(
    const double boxcenter[3],
    double boxhalfsize[3],
    const double* pt1,
    const double* pt2,
    const double* pt3);

/* inline functions */
// #include <malloc.h>
// static inline
// double memory_usage(int mpi_rank,int timestep,int display){
//
//     /* get malloc info structure */
//     struct mallinfo my_mallinfo = mallinfo();
//
//     /*total memory reserved by the system for malloc currently */
//     double reserved_mem = my_mallinfo.arena;
//
//     /* get all the memory currently allocated to user by malloc, etc. */
//     double used_mem = my_mallinfo.hblkhd
//                     + my_mallinfo.usmblks
//                     + my_mallinfo.uordblks;
//
//     /* get memory not currently allocated to user but malloc controls */
//     double free_mem = my_mallinfo.fsmblks
//                     + my_mallinfo.fordblks;
//
//     /* get number of items currently allocated */
//     /* double number_allocated = my_mallinfo.ordblks + my_mallinfo.smblks; */
//
//     /* Print out concise malloc info line */
//     if(display && mpi_rank == 0){
//         printf("Step[%d]: %f MB(%.0f) malloc: %f MB reserved (%.0f
//         unused)\n",
//             timestep,
//             used_mem / (1024.0 * 1024.0),
//             used_mem,
//             reserved_mem / (1024.0 * 1024.0),
//             free_mem);
//
//         if(mpi_rank == 0){
//             FILE *fp;
//             char filename[] = "tiogaMemUsage.dat";
//             fp=fopen(filename,"a");
//             fprintf(fp,"Step[%d]: %f MB(%.0f) malloc: %f MB reserved (%.0f
//             unused)\n",
//                     timestep,used_mem / (1024.0 *
//                     1024.0),used_mem,reserved_mem / (1024.0 * 1024.0),
//                     free_mem);
//             fclose(fp);
//         }
//
//     }
//     return used_mem;
// }

#endif /* TIOGA_UTILS_H */
