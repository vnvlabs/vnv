

from mpi4py import MPI
import numpy as np
import adios2
comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

if( rank == 0 ):
    # with-as will call adios2.close on fh at the end
    # if only one rank is active pass MPI.COMM_SELF
    with adios2.open("output_default.bp", "r", MPI.COMM_SELF) as fh:
    for fh_step in fh:
        step = fh_step.currentstep()
        if( step == 0 ):
            size_in = fh_step.read("size")
        physical_time = fh_step.read("physical_time")
        temperature = fh_step.read("temperature", start, count)
        pressure = fh_step.read("pressure", start, count)
