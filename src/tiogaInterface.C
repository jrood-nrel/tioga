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
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include "mpi.h"
#include "codetypes.h"
#include "tioga.h"
#include "globals.h"
//
// All the interfaces that are
// accessible to third party f90 and C
// flow solvers
//
//
// Jay Sitaraman
// 02/24/2014
//
extern "C" {

void tioga_init_f90_(const int* scomm)
{
    int id_proc, nprocs;
    MPI_Comm tcomm;
    // tcomm=(MPI_Comm) (*scomm);
    tcomm = MPI_Comm_f2c(*scomm);
    //
    tg = new tioga[1];
    //
    // MPI_Comm_rank(MPI_COMM_WORLD,&id_proc);
    // MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
    MPI_Comm_rank(tcomm, &id_proc);
    MPI_Comm_size(tcomm, &nprocs);
    //
    tg->setCommunicator(tcomm, id_proc, nprocs);
    for (auto& i : idata) {
        i.nc = nullptr;
        i.nv = nullptr;
        i.vconn = nullptr;
    }
}

void tioga_init_(MPI_Comm tcomm)
{
    int id_proc, nprocs;
    // MPI_Comm tcomm;
    // tcomm=(MPI_Comm) (*scomm);
    // tcomm=MPI_Comm_f2c(*scomm);
    //
    tg = new tioga[1];
    //
    // MPI_Comm_rank(MPI_COMM_WORLD,&id_proc);
    // MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
    MPI_Comm_rank(tcomm, &id_proc);
    MPI_Comm_size(tcomm, &nprocs);
    //
    tg->setCommunicator(tcomm, id_proc, nprocs);
    for (auto& i : idata) {
        i.nc = nullptr;
        i.nv = nullptr;
        i.vconn = nullptr;
    }
}

void tioga_registergrid_data_(
    const int* btag,
    const int* nnodes,
    double* xyz,
    int* ibl,
    const int* nwbc,
    const int* nobc,
    int* wbcnode,
    int* obcnode,
    int* ntypes,
    ...)
{
    va_list arguments;
    int i;
    int const iblk = 0;
    va_start(arguments, ntypes);

    if (idata[iblk].nv != nullptr) TIOGA_FREE(idata[iblk].nv);
    if (idata[iblk].nc != nullptr) TIOGA_FREE(idata[iblk].nc);
    if (idata[iblk].vconn != nullptr) TIOGA_FREE(idata[iblk].vconn);
    idata[iblk].nv = (int*)malloc(sizeof(int) * (*ntypes));
    idata[iblk].nc = (int*)malloc(sizeof(int) * (*ntypes));
    idata[iblk].vconn = (int**)malloc(sizeof(int*) * (*ntypes));
    for (i = 0; i < *ntypes; i++) {
        idata[iblk].nv[i] = *(va_arg(arguments, int*));
        idata[iblk].nc[i] = *(va_arg(arguments, int*));
        idata[iblk].vconn[i] = va_arg(arguments, int*);
    }
    tg->registerGridData(
        *btag, *nnodes, xyz, ibl, *nwbc, *nobc, wbcnode, obcnode, *ntypes,
        idata[iblk].nv, idata[iblk].nc, idata[iblk].vconn);
}

void tioga_registergrid_data_mb_(
    const int* bid,
    const int* btag,
    const int* nnodes,
    double* xyz,
    int* ibl,
    const int* nwbc,
    const int* nobc,
    int* wbcnode,
    int* obcnode,
    int* ntypes,
    ...)
{
    va_list arguments;
    int i;
    int const iblk = *bid - BASE;

    va_start(arguments, ntypes);

    if (idata[iblk].nv != nullptr) TIOGA_FREE(idata[iblk].nv);
    if (idata[iblk].nc != nullptr) TIOGA_FREE(idata[iblk].nc);
    if (idata[iblk].vconn != nullptr) TIOGA_FREE(idata[iblk].vconn);
    idata[iblk].nv = (int*)malloc(sizeof(int) * (*ntypes));
    idata[iblk].nc = (int*)malloc(sizeof(int) * (*ntypes));
    idata[iblk].vconn = (int**)malloc(sizeof(int*) * (*ntypes));
    for (i = 0; i < *ntypes; i++) {
        idata[iblk].nv[i] = *(va_arg(arguments, int*));
        idata[iblk].nc[i] = *(va_arg(arguments, int*));
        idata[iblk].vconn[i] = va_arg(arguments, int*);
    }
    tg->registerGridData(
        *btag, *nnodes, xyz, ibl, *nwbc, *nobc, wbcnode, obcnode, *ntypes,
        idata[iblk].nv, idata[iblk].nc, idata[iblk].vconn);
}

void tioga_register_amr_global_data_(
    const int* nf, int* idata, double* rdata, const int* ngridsin)
{
    tg->register_amr_global_data(*nf, idata, rdata, *ngridsin);
}

void tioga_register_amr_patch_count_(const int* npatches)
{
    tg->set_amr_patch_count(*npatches);
}

void tioga_register_amr_local_data_(
    const int* ipatch, const int* global_id, int* iblank, int* iblankn)
{
    tg->register_amr_local_data(*ipatch, *global_id, iblank, iblankn);
}

void tioga_register_amr_solution_(
    const int* ipatch, double* q, int nvar_cell, int nvar_node)
{
    tg->register_amr_solution(*ipatch, q, nvar_cell, nvar_node);
}

void tioga_setcomposite_(const int* ncomp)
{
    tg->setNumCompositeBodies(*ncomp);
}

void tioga_register_composite_body_(
    const int* compbodytag,
    const int* nbodytags,
    int* bodytags,
    int* dominancetags,
    const double* searchTol)
{
    tg->registerCompositeBody(
        *compbodytag, *nbodytags, bodytags, dominancetags, *searchTol);
}

void tioga_preprocess_grids_(void)
{
    if (tg->getHoleMapAlgorithm() == 1) {
        tg->assembleComplementComms(); // build complement rank communicators
        if (tg->getNumCompositeBodies() > 0) {
            tg->assembleCompositeMap(); // abutting meshes (AFTER
        }
        // assembleComplementComms)
    }
    tg->profile();
}

void tioga_performconnectivity_(void) { tg->performConnectivity(); }

void tioga_performconnectivity_highorder_(void)
{
    tg->performConnectivityHighOrder();
}

void tioga_performconnectivity_amr_(void) { tg->performConnectivityAMR(); }

void tioga_registersolution_(const int* bid, double* q)
{
    tg->registerSolution(*bid, q);
}

void tioga_dataupdate_mb_(const int* nvar, char* itype)
{
    int interptype;
    if (strstr(itype, "row") != nullptr) {
        interptype = 0;
    } else if (strstr(itype, "column") != nullptr) {
        interptype = 1;
    } else {
        printf("#tiogaInterface.C:dataupdate_:unknown data orientation\n");
        return;
    }

    // tg->dataUpdate(*nvar,interptype);

    if (tg->ihighGlobal == 0) {
        if (tg->iamrGlobal == 0) {
            tg->dataUpdate(*nvar, interptype);
        } else {
            tg->dataUpdate_AMR();
        }
    } else {
        if (tg->iamrGlobal == 0) {
            tg->dataUpdate(*nvar, interptype, 1);
        } else {
            printf(
                "Data update between high-order near-body and AMR cartesian "
                "Not implemented yet\n");
        }
    }
}

void tioga_dataupdate_(double* q, int* nvar, char* itype)
{
    int interptype;
    int const bid = 0;
    tg->registerSolution(bid, q);
    tioga_dataupdate_mb_(nvar, itype);
}

void tioga_writeoutputfiles_(const int* nvar, char* itype)
{
    int interptype;
    if (strstr(itype, "row") != nullptr) {
        interptype = 0;
    } else if (strstr(itype, "column") != nullptr) {
        interptype = 1;
    } else {
        printf("#tiogaInterface.C:dataupdate_:unknown data orientation\n");
        return;
    }
    tg->writeData(*nvar, interptype);
}
void tioga_getdonorcount_(const int* btag, int* dcount, int* fcount)
{
    tg->getDonorCount(*btag, dcount, fcount);
}
void tioga_getdonorinfo_(
    const int* btag, int* receptors, int* indices, double* frac, int* dcount)
{
    tg->getDonorInfo(*btag, receptors, indices, frac, dcount);
}

void tioga_setholemapalg_(const int* alg) { tg->setHoleMapAlgorithm(*alg); }

void tioga_setsymmetry_(const int* isym) { tg->setSymmetry(*isym); }

void tioga_setresolutions_(double* nres, double* cres)
{
    tg->setResolutions(nres, cres);
}

void tioga_setresolutions_multi_(const int* btag, double* nres, double* cres)
{
    tg->setResolutions(*btag, nres, cres);
}

void tioga_setcelliblank_(int* iblank_cell)
{
    tg->set_cell_iblank(iblank_cell);
}

void tioga_set_highorder_callback_(
    void (*f1)(int*, int*),
    void (*f2)(int*, int*, double*),
    void (*f3)(int*, double*, int*, double*),
    void (*f4)(int*, double*, int*, int*, double*, double*, int*),
    void (*f5)(int*, int*, double*, int*, int*, double*))
{
    tg->setcallback(f1, f2, f3, f4, f5);
    //    get_nodes_per_cell=f1;
    // get_receptor_nodes=f2;
    // donor_inclusion_test=f3;
    // donor_frac=f4;
    // convert_to_modal=f5;
}

void tioga_set_p4est_(void) { tg->set_p4est(); }
void tioga_set_amr_callback_(void (*f1)(int*, double*, int*, double*))
{
    tg->set_amr_callback(f1);
}
void tioga_set_p4est_search_callback_(
    void (*f1)(double* xsearch, int* process_id, int* cell_id, int* npts),
    void (*f2)(int* pid, int* iflag))
{
    tg->setp4estcallback(f1, f2);
    // jayfixme  tg->set_p4est_search_callback(f1);
}

void tioga_reduce_fringes_(void) { tg->reduce_fringes(); }

void tioga_setnfringe_(int* nfringe) { tg->setNfringe(nfringe); }

void tioga_setmexclude_(int* mexclude) { tg->setMexclude(mexclude); }

void tioga_delete_(void)
{
    delete[] tg;
    for (auto& i : idata) {
        if (i.nc != nullptr) TIOGA_FREE(i.nc);
        if (i.nv != nullptr) TIOGA_FREE(i.nv);
        if (i.vconn != nullptr) TIOGA_FREE(i.vconn);
    }
}
}
