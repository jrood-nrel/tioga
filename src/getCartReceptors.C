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
//
// routine for extra query points
// have to unify both routines here
// FIX later ...

#include <cstdlib>
#include <cmath>
#include "codetypes.h"
#include "MeshBlock.h"
#include "parallelComm.h"
#include "CartGrid.h"
#include "cartUtils.h"
#include "linklist.h"
#include "tioga_utils.h"

void MeshBlock::getCartReceptors(CartGrid* cg, parallelComm* pc)
{
    INTEGERLIST2* head;
    INTEGERLIST2* dataPtr;
    //
    // limit case we communicate to everybody
    //
    int* pmap = (int*)malloc(sizeof(int) * pc->numprocs);
    for (int i = 0; i < pc->numprocs; i++) {
        pmap[i] = 0;
    }
    //
    OBB* obcart = (OBB*)malloc(sizeof(OBB));
    for (auto& j : obcart->vec) {
        for (double& k : j) {
            k = 0;
        }
    }
    obcart->vec[0][0] = obcart->vec[1][1] = obcart->vec[2][2] = 1.0;
    //
    head = (INTEGERLIST2*)malloc(sizeof(INTEGERLIST2));
    head->intData = nullptr;
    head->realData = nullptr;
    dataPtr = head;
    dataPtr->next = nullptr;
    //
    nsearch = 0;
    //
    for (int c = 0; c < cg->ngrids; c++) {
        int const cell_count =
            (cg->dims[static_cast<int>(3 * c)] + 2 * cg->nf) *
            (cg->dims[3 * c + 1] + 2 * cg->nf) *
            (cg->dims[3 * c + 2] + 2 * cg->nf);

        int const vol = cg->dx[static_cast<int>(3 * c)] * cg->dx[3 * c + 1] *
                        cg->dx[3 * c + 2];

        for (int n = 0; n < 3; n++) {
            obcart->dxc[n] = cg->dx[3 * c + n] * (cg->dims[3 * c + n]) * 0.5;
            obcart->xc[n] = cg->xlo[3 * c + n] + obcart->dxc[n];
        }

        int intersectCount = 0;
        if ((obbIntersectCheck(
                 obb->vec, obb->xc, obb->dxc, obcart->vec, obcart->xc,
                 obcart->dxc) != 0) ||
            (obbIntersectCheck(
                 obcart->vec, obcart->xc, obcart->dxc, obb->vec, obb->xc,
                 obb->dxc) != 0)) {
            intersectCount++;

            auto* xtm = (double*)malloc(sizeof(double) * 3);

            for (int j = 0; j < cg->dims[static_cast<int>(3 * c)]; j++) {
                for (int k = 0; k < cg->dims[3 * c + 1]; k++) {
                    for (int l = 0; l < cg->dims[3 * c + 2]; l++) {
                        fillReceptorDataPtr(
                            cg, cell_count, c, j, k, l, pmap, vol, xtm, false,
                            dataPtr);
                    }
                }
            }

            for (int j = 0; j < cg->dims[static_cast<int>(3 * c)] + 1; j++) {
                for (int k = 0; k < cg->dims[3 * c + 1] + 1; k++) {
                    for (int l = 0; l < cg->dims[3 * c + 2] + 1; l++) {
                        fillReceptorDataPtr(
                            cg, cell_count, c, j, k, l, pmap, vol, xtm, true,
                            dataPtr);
                    }
                }
            }

            TIOGA_FREE(xtm);
        }
    }
    //
    // create the communication map
    //
    int nsend = 0;
    for (int i = 0; i < pc->numprocs; i++) {
        if (pmap[i] == 1) {
            nsend++;
        }
    }
    int const nrecv = nsend;
    int* sndMap = (int*)malloc(sizeof(int) * nsend);
    int* rcvMap = (int*)malloc(sizeof(int) * nrecv);
    int m = 0;
    for (int i = 0; i < pc->numprocs; i++) {
        if (pmap[i] == 1) {
            sndMap[m] = rcvMap[m] = i;
            m++;
        }
    }
    pc->setMap(nsend, nrecv, sndMap, rcvMap);
    //
    // if these were already allocated
    // get rid of them
    //
    if (donorId != nullptr) TIOGA_FREE(donorId);
    if (tagsearch != nullptr) TIOGA_FREE(tagsearch);
    if (isearch != nullptr) TIOGA_FREE(isearch);
    if (rst != nullptr) TIOGA_FREE(rst);
    if (res_search != nullptr) TIOGA_FREE(res_search);
    if (xsearch != nullptr) TIOGA_FREE(xsearch);
    //
    donorId = (int*)malloc(sizeof(int) * nsearch);
    tagsearch = (int*)malloc(sizeof(int) * nsearch);
    isearch = (int*)malloc(3 * sizeof(int) * nsearch);
    res_search = (double*)malloc(sizeof(double) * nsearch);
    xsearch = (double*)malloc(sizeof(double) * 3 * nsearch);
    rst = (double*)malloc(sizeof(double) * 3 * nsearch);
    //
    dataPtr = head->next;
    int k = 0, l = 0, n = 0, p = 0;
    while (dataPtr != nullptr) {
        for (int j = 0; j < dataPtr->intDataSize - 1; j++) {
            isearch[p++] = dataPtr->intData[j];
        }
        tagsearch[k++] = dataPtr->intData[dataPtr->intDataSize - 1];

        for (int j = 0; j < dataPtr->realDataSize - 1; j++) {
            xsearch[n++] = dataPtr->realData[j];
        }
        res_search[l++] = dataPtr->realData[dataPtr->realDataSize - 1];

        dataPtr = dataPtr->next;
    }

    deallocateLinkList3(head);
    // fclose(fp);
    TIOGA_FREE(obcart);
    TIOGA_FREE(pmap);
    TIOGA_FREE(sndMap);
    TIOGA_FREE(rcvMap);
}

void MeshBlock::fillReceptorDataPtr(
    CartGrid* cg,
    int /*cell_count*/,
    int c,
    int j,
    int k,
    int l,
    int* pmap,
    double vol,
    double* xtm,
    bool isNodal,
    INTEGERLIST2*& dataPtr)
{
    int itm = -1;
    if (isNodal) {
        itm = cart_utils::get_concatenated_node_index(
            cg->dims[static_cast<int>(3 * c)], cg->dims[3 * c + 1],
            cg->dims[3 * c + 2], cg->nf, j, k, l);

        xtm[0] = cg->xlo[static_cast<int>(3 * c)] +
                 j * cg->dx[static_cast<int>(3 * c)];
        xtm[1] = cg->xlo[3 * c + 1] + k * cg->dx[3 * c + 1];
        xtm[2] = cg->xlo[3 * c + 2] + l * cg->dx[3 * c + 2];
    } else {
        itm = cart_utils::get_cell_index(
            cg->dims[static_cast<int>(3 * c)], cg->dims[3 * c + 1], cg->nf, j,
            k, l);

        xtm[0] = cg->xlo[static_cast<int>(3 * c)] +
                 (j + 0.5) * cg->dx[static_cast<int>(3 * c)];
        xtm[1] = cg->xlo[3 * c + 1] + (k + 0.5) * cg->dx[3 * c + 1];
        xtm[2] = cg->xlo[3 * c + 2] + (l + 0.5) * cg->dx[3 * c + 2];
    }

    double xd[3];
    for (double& jj : xd) {
        jj = 0;
    }
    for (int jj = 0; jj < 3; jj++) {
        for (int kk = 0; kk < 3; kk++) {
            xd[jj] += (xtm[kk] - obb->xc[kk]) * obb->vec[jj][kk];
        }
    }

    int iflag = 0;
    if (fabs(xd[0]) <= obb->dxc[0] && fabs(xd[1]) <= obb->dxc[1] &&
        fabs(xd[2]) <= obb->dxc[2]) {
        iflag++;
    }

    if (iflag > 0) {
        pmap[cg->proc_id[c]] = 1;
        dataPtr->next = (INTEGERLIST2*)malloc(sizeof(INTEGERLIST2));
        dataPtr = dataPtr->next;
        dataPtr->intDataSize = 4;
        dataPtr->realDataSize = 4;
        dataPtr->realData =
            (double*)malloc(sizeof(double) * dataPtr->realDataSize);
        dataPtr->intData = (int*)malloc(sizeof(int) * dataPtr->intDataSize);
        dataPtr->intData[0] = cg->proc_id[c];
        dataPtr->intData[1] = cg->local_id[c];
        dataPtr->intData[2] = itm;
        dataPtr->intData[3] = 0;
        nsearch += 1;

        for (int kk = 0; kk < dataPtr->realDataSize - 1; kk++) {
            dataPtr->realData[kk] = xtm[kk];
        }

        dataPtr->realData[dataPtr->realDataSize - 1] = vol;

        dataPtr->next = nullptr;
    }
}
