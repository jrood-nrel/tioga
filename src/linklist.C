
/* This file is part of the Tioga software library */

/* Tioga  is a tool for overset grid assembly on parallel distributed systems */
/* Copyright (C) 2015 Jay Sitaraman */

/* This library is free software; you can redistribute it and/or */
/* modify it under the terms of the GNU Lesser General Public */
/* License as published by the Free Software Foundation; either */
/* version 2.1 of the License, or (at your option) any later version. */

/* This library is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU */
/* Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public */
/* License along with this library; if not, write to the Free Software */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */
#include <cmath>
#include "codetypes.h"
#include "linklist.h"

void deallocateLinkList(DONORLIST* temp)
{
    while (temp != nullptr) {
        DONORLIST* next = temp->next;
        TIOGA_FREE(temp);
        temp = next;
    }
}

void deallocateLinkList2(INTEGERLIST* temp)
{
    while (temp != nullptr) {
        INTEGERLIST* next = temp->next;
        TIOGA_FREE(temp);
        temp = next;
    }
}

void deallocateLinkList3(INTEGERLIST2* temp)
{
    while (temp != nullptr) {
        INTEGERLIST2* tnext = temp->next;
        if (temp->intData != nullptr) TIOGA_FREE(temp->intData);
        if (temp->realData != nullptr) TIOGA_FREE(temp->realData);
        TIOGA_FREE(temp);
        temp = tnext;
    }
}

void deallocateLinkList4(INTERPLIST2* temp)
{
    while (temp != nullptr) {
        INTERPLIST2* tnext = temp->next;
        if (temp->inode != nullptr) TIOGA_FREE(temp->inode);
        if (temp->weights != nullptr) TIOGA_FREE(temp->weights);
        TIOGA_FREE(temp);
        temp = tnext;
    }
}

void insertInList(DONORLIST** donorList, DONORLIST* temp1)
{
    DONORLIST* temp;
    DONORLIST* ptemp;
    int inserted;
    temp = *donorList;
    inserted = 0;
    ptemp = nullptr;
    while (temp != nullptr && (inserted == 0)) {
        if (fabs(temp->donorRes) > temp1->donorRes) {
            temp1->next = temp;
            if (ptemp != nullptr) {
                ptemp->next = temp1;
            } else {
                *donorList = temp1;
            }
            inserted = 1;
        } else {
            ptemp = temp;
            temp = temp->next;
        }
    }
    if (inserted == 0) {
        if (*donorList != nullptr) {
            temp1->next = nullptr;
            ptemp->next = temp1;
        } else {
            temp1->next = nullptr;
            *donorList = temp1;
        }
    }
}
