#Set amrex options
set(USE_XSDK_DEFAULTS OFF)
set(AMReX_SPACEDIM 3)
set(AMReX_AMRDATA OFF)
set(AMReX_ASSERTIONS OFF)
set(AMReX_ASCENT OFF)
set(AMReX_BACKTRACE OFF)
set(AMReX_BASE_PROFILE OFF)
set(AMReX_COMM_PROFILE OFF)
set(AMReX_CONDUIT OFF)
set(AMReX_CUDA ${AMR_WIND_ENABLE_CUDA})
set(AMReX_PRECISION DOUBLE)
set(AMReX_DPCPP ${AMR_WIND_ENABLE_DPCPP})
set(AMReX_EB OFF)
set(AMReX_FORTRAN ${AMR_WIND_ENABLE_FORTRAN})
set(AMReX_FORTRAN_INTERFACES ${AMR_WIND_ENABLE_FORTRAN})
set(AMReX_FPE OFF)
set(AMReX_HIP ${AMR_WIND_ENABLE_HIP})
set(AMReX_HYPRE ${AMR_WIND_ENABLE_HYPRE})
set(AMReX_LINEAR_SOLVERS ON)
set(AMReX_MEM_PROFILE OFF)
set(AMReX_MPI ${AMR_WIND_ENABLE_MPI})
set(AMReX_OMP ${AMR_WIND_ENABLE_OPENMP})
set(AMReX_PARTICLES ON)
set(AMReX_PIC ON)
set(AMReX_PLOTFILE_TOOLS ${AMR_WIND_ENABLE_FCOMPARE})
set(AMReX_PROFPARSER OFF)
set(AMReX_SENSEI OFF)
set(AMReX_SUNDIALS OFF)
set(AMReX_TINY_PROFILE OFF)
set(AMReX_TRACE_PROFILE OFF)
