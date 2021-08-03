#include "VnV.h"

/***
 * Tests for the communicatoin api on 4 nodes.
 */
#define COMMTESTSIZE

#define RANK(root,...) if (rank == root) { __VA_ARGS__ }

INJECTION_UNITTEST(VnVTestRunner,Communication,4) {

    int size = comm->Size();
    int rank = comm->Rank();
    if (rank == 0) {
       TEST_ASSERT_EQUALS("size",size,4);
    }

    std::vector<int> data(4);
    if (rank == 3) {
      data = { 1 , 2 , 3, 4 };
    }
    comm->BroadCast(&data[0],data.size(),sizeof(int),3);
    if (rank == 0 ) {
        bool workd = (data[0] == 1 && data[1] == 2 && data[2] == 3 && data[3] == 4 );
       TEST_ASSERT_EQUALS("bcast from 3, read at 0", workd,true);
    }

    bool rworkd = false;
    bool workd = (rank == 0 ) ? false : true;
    comm->Reduce(&workd,1,&rworkd,sizeof(bool), VnV::OpType::LAND, 0 );
    if (rank == 0 ) {
       TEST_ASSERT_EQUALS("reduce LAND", rworkd, false);
    }
    comm->Reduce(&workd,1,&rworkd,sizeof(bool), VnV::OpType::LOR, 0 );
    if (rank == 0 ) {
       TEST_ASSERT_EQUALS("reduce LOR", rworkd, true);
    }
    double d = 10.2;
    double s = -1;
    comm->Reduce(&d,1,&s,sizeof(double), VnV::OpType::SUM, 0 );
    if (rank == 0 ) {
       TEST_ASSERT_EQUALS("reduce SUM", d*size , s );
    }
    std::vector<int> ranks = {rank};

    bool cc =  comm->contains(comm->create(ranks,10));
    RANK(0,TEST_ASSERT_EQUALS("comm contains", true, cc));

    cc = comm->create(ranks,10)->contains(comm);
    RANK(0,TEST_ASSERT_EQUALS("comm not contained in ", false, cc));


    bool cd = VnV::CommCompareType::EXACT ==  comm->compare(comm);
    RANK(0,TEST_ASSERT_EQUALS("Comm compare with self", true, cd));

    cd = VnV::CommCompareType::GROUP ==  comm->compare(comm->duplicate());
    RANK(0,TEST_ASSERT_EQUALS("Comm compare with duplicate of self", true, cd));

    cd = VnV::CommCompareType::UNEQUAL ==  comm->compare(comm->create(ranks,10));
    RANK(0,TEST_ASSERT_EQUALS("Comm compare with different comm", true, cd));

    std::vector<int> re(4);
    comm->AllReduce(&data[0],2,&re[0],sizeof(int),VnV::OpType::PROD);
    cc = (re[0] = 1 && re[1] == 16 );
    RANK(0, TEST_ASSERT_EQUALS("All Reduce", true, cc);)

    comm->Gather(&data[rank],1,&re[0],sizeof(int),0);
    cc = (re[0] = 1 && re[3] == 4 );
    RANK(0, TEST_ASSERT_EQUALS("Gather", true, cc);)

    if (rank == 1 ) {
       comm->Send(&data[1],1,0,0,sizeof(int));
    } else if (rank == 0 ) {
       comm->Recv(&re[0],1,1,0,sizeof(int));
       TEST_ASSERT_EQUALS("Send And Recv", 2, re[0] );
    }

    if (rank == 0) {
      re[1] = -10;
      comm->Wait(comm->IRecv(&re[1],1,1,1,sizeof(int)));
      TEST_ASSERT_EQUALS("Send IRecv and Wait", 2, re[1] );
    } else if ( rank == 1 ) {
      comm->Send(&data[1],1,0,1, sizeof(int));
    }

    if (rank == 0) {
      re[1] = -10;
      VnV::IStatus_ptr ptr = comm->Probe(1,1);
      TEST_ASSERT_EQUALS("Probe Tag" , 1, ptr->tag());
      TEST_ASSERT_EQUALS("Probe Source", 1, ptr->source());
      comm->Recv(&re[1],1,1,1,sizeof(int));
      TEST_ASSERT_EQUALS("ISend -> Probe -> Recv ", 2, re[1] );
    } else if ( rank == 1 ) {
      comm->Wait(comm->ISend(&data[1],1,0,1, sizeof(int)));
    }


}
