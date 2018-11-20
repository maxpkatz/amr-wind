#include <incflo.H>

void incflo::AllocateArrays(int lev)
{
    UpdateEBFactory(lev);

	// ********************************************************************************
	// Cell- or node-based arrays
	// ********************************************************************************

    // Gas density
    ro[lev].reset(new MultiFab(grids[lev], dmap[lev], 1, nghost));
	ro[lev]->setVal(0.);

    // Pressure
    if (nodal_pressure)
    {
        const BoxArray & nd_grids = amrex::convert(grids[lev], IntVect{1,1,1});
        p0[lev].reset(new MultiFab(nd_grids, dmap[lev], 1, nghost));
        p[lev].reset(new MultiFab(nd_grids, dmap[lev], 1, nghost));
    } 
    else 
    {
        p0[lev].reset(new MultiFab(grids[lev], dmap[lev], 1, nghost));
        p[lev].reset(new MultiFab(grids[lev], dmap[lev], 1, nghost));
    }
	p0[lev]->setVal(0.);
	p[lev]->setVal(0.);

	// Pressure gradients
	gp[lev].reset(new MultiFab(grids[lev], dmap[lev], 3, nghost));
	gp0[lev].reset(new MultiFab(grids[lev], dmap[lev], 3, nghost));
	gp[lev]->setVal(0.);
	gp0[lev]->setVal(0.);

	// Viscosity
	eta[lev].reset(new MultiFab(grids[lev], dmap[lev], 1, nghost));
	eta[lev]->setVal(0.);

	// Current velocity
	vel[lev].reset(new MultiFab(grids[lev], dmap[lev], 3, nghost, MFInfo(), *ebfactory[lev]));
	vel[lev]->setVal(0.);

	// Old velocity
	vel_o[lev].reset(new MultiFab(grids[lev], dmap[lev], 3, nghost, MFInfo(), *ebfactory[lev]));
	vel_o[lev]->setVal(0.);

	// Strain-rate magnitude
	strainrate[lev].reset(new MultiFab(grids[lev], dmap[lev], 1, nghost));
	strainrate[lev]->setVal(0.);

	// Vorticity
	vort[lev].reset(new MultiFab(grids[lev], dmap[lev], 1, nghost));
	vort[lev]->setVal(0.);

	// Arrays to store the solution and rhs for the projection
    if (nodal_pressure)
    {
        const BoxArray& nd_grids = amrex::convert(grids[lev], IntVect{1,1,1});
        phi[lev].reset(new MultiFab(nd_grids, dmap[lev], 1, nghost, MFInfo(), *ebfactory[lev]));
        divu[lev].reset(new MultiFab(nd_grids, dmap[lev], 1, nghost, MFInfo(), *ebfactory[lev]));
    }
    else
    {
        phi[lev].reset(new MultiFab(grids[lev], dmap[lev], 1, nghost, MFInfo(), *ebfactory[lev]));
        divu[lev].reset(new MultiFab(grids[lev], dmap[lev], 1, nghost, MFInfo(), *ebfactory[lev]));
    }
	phi[lev]->setVal(0.);
	divu[lev]->setVal(0.);

	// Slopes in x-direction
	xslopes[lev].reset(new MultiFab(grids[lev], dmap[lev], 3, nghost));
	xslopes[lev]->setVal(0.);

	// Slopes in y-direction
	yslopes[lev].reset(new MultiFab(grids[lev], dmap[lev], 3, nghost));
	yslopes[lev]->setVal(0.);

	// Slopes in z-direction
	zslopes[lev].reset(new MultiFab(grids[lev], dmap[lev], 3, nghost));
	zslopes[lev]->setVal(0.);

	// ********************************************************************************
	// X-face-based arrays
	// ********************************************************************************

    // When the pressure is on nodes, bcoeff is at cell centers
    for(int dir = 0; dir < 3; dir++)
    {
        if (nodal_pressure)
        {
            bcoeff[lev][dir].reset(new MultiFab(grids[lev], dmap[lev], 1, nghost));
        }
        else
        {
            // Create a BoxArray on x-faces.
            BoxArray edge_ba = grids[lev];
            edge_ba.surroundingNodes(dir);
            bcoeff[lev][dir].reset(new MultiFab(edge_ba, dmap[lev], 1, nghost, 
                                              MFInfo(), *ebfactory[lev]));
        }
        bcoeff[lev][dir]->setVal(0.);
    }

	// ****************************************************************

	// Create a BoxArray on x-faces.
    BoxArray x_edge_ba = grids[lev].surroundingNodes(0);
	m_u_mac[lev].reset(new MultiFab(x_edge_ba, dmap[lev], 1, nghost, MFInfo(), *ebfactory[lev]));
	m_u_mac[lev]->setVal(0.);

	// Create a BoxArray on y-faces.
    BoxArray y_edge_ba = grids[lev].surroundingNodes(1);
	m_v_mac[lev].reset(new MultiFab(y_edge_ba, dmap[lev], 1, nghost, MFInfo(), *ebfactory[lev]));
	m_v_mac[lev]->setVal(0.);

	// Create a BoxArray on y-faces.
    BoxArray z_edge_ba = grids[lev].surroundingNodes(2);
	m_w_mac[lev].reset(new MultiFab(z_edge_ba, dmap[lev], 1, nghost, MFInfo(), *ebfactory[lev]));
	m_w_mac[lev]->setVal(0.);
}

void incflo::RegridArrays(int lev)
{
    UpdateEBFactory(lev);

	// ********************************************************************************
	// Cell-based arrays
	// ********************************************************************************
    //
    // After calling copy() with dst_ngrow set to ng, we do not need to call
    // FillBoundary().
    //
    //

	// Gas density
	std::unique_ptr<MultiFab> ro_new(new MultiFab(grids[lev], dmap[lev], 1, nghost));
	ro_new->setVal(0.0);
	ro_new->copy(*ro[lev], 0, 0, 1, 0, nghost);
	ro[lev] = std::move(ro_new);

    // Pressures, projection vars
    if (nodal_pressure)
    {
        const BoxArray & nd_grids = amrex::convert(grids[lev], IntVect{1,1,1});

        std::unique_ptr<MultiFab> p_new(new MultiFab(nd_grids, dmap[lev], 1, nghost));
        p_new->setVal(0.0);
        p_new->copy(*p[lev],0,0,1,0,nghost);
        p[lev] = std::move(p_new);

        std::unique_ptr<MultiFab> p0_new(new MultiFab(nd_grids, dmap[lev], 1, nghost));
        p0_new->setVal(0.0);
        p0_new->copy(*p0[lev],0,0,1,0,nghost);
        p0[lev] = std::move(p0_new);

        std::unique_ptr<MultiFab> divu_new(new MultiFab(nd_grids, dmap[lev], 1, nghost));
        divu[lev] = std::move(divu_new);
        divu[lev]->setVal(0.);

        std::unique_ptr<MultiFab> phi_new(new MultiFab(nd_grids, dmap[lev], 1, nghost));
        phi[lev] = std::move(phi_new);
        phi[lev]->setVal(0.);

        std::unique_ptr<MultiFab> bc0_new(new MultiFab(grids[lev], dmap[lev], 1, nghost, 
                                                       MFInfo(), *ebfactory[lev] ));
        bcoeff[lev][0] = std::move(bc0_new);
        bcoeff[lev][0]->setVal(0.);

        std::unique_ptr<MultiFab> bc1_new(new MultiFab(grids[lev], dmap[lev], 1, nghost,
                                                       MFInfo(), *ebfactory[lev]));
        bcoeff[lev][1] = std::move(bc1_new);
        bcoeff[lev][1]->setVal(0.);

        std::unique_ptr<MultiFab> bc2_new(new MultiFab(grids[lev], dmap[lev], 1, nghost,
                                                       MFInfo(), *ebfactory[lev]));
        bcoeff[lev][2] = std::move(bc2_new);
        bcoeff[lev][2]->setVal(0.);

    }
    else
    {
        std::unique_ptr<MultiFab> p_new(new MultiFab(grids[lev], dmap[lev], 1, nghost));
        p_new->setVal(0.);
        p_new->copy(*p[lev], 0, 0, 1, 0, nghost);
        p[lev] = std::move(p_new);

        std::unique_ptr<MultiFab> p0_new(new MultiFab(grids[lev], dmap[lev], 1, nghost));
        p0_new->setVal(0.);
        p0_new->copy(*p0[lev], 0, 0, 1, 0, nghost);
        p0[lev] = std::move(p0_new);

        std::unique_ptr<MultiFab> phi_new(new MultiFab(grids[lev], dmap[lev], 1, nghost, 
                                                       MFInfo(), *ebfactory[lev]));
        phi[lev] = std::move(phi_new);
        phi[lev]->setVal(0.);

        std::unique_ptr<MultiFab> divu_new(new MultiFab(grids[lev], dmap[lev], 1, nghost, 
                                           MFInfo(), *ebfactory[lev]));
        divu[lev] = std::move(divu_new);
        divu[lev]->setVal(0.);

        // Cell-centered pressure uses face-based coefficients
        BoxArray x_ba = grids[lev];
        x_ba = x_ba.surroundingNodes(0);
        std::unique_ptr<MultiFab> bc0_new(new MultiFab(x_ba, dmap[lev], 1, nghost, 
                                                       MFInfo(), *ebfactory[lev]));
        bcoeff[lev][0] = std::move(bc0_new);
        bcoeff[lev][0]->setVal(0.0);

        BoxArray y_ba = grids[lev];
        y_ba = y_ba.surroundingNodes(1);
        std::unique_ptr<MultiFab> bc1_new(new MultiFab(y_ba, dmap[lev], 1, nghost, 
                                                       MFInfo(), *ebfactory[lev]));
        bcoeff[lev][1] = std::move(bc1_new);
        bcoeff[lev][1]->setVal(0.0);

        BoxArray z_ba = grids[lev];
        z_ba = z_ba.surroundingNodes(2);
        std::unique_ptr<MultiFab> bc2_new(new MultiFab(z_ba, dmap[lev], 1, nghost, 
                                                       MFInfo(), *ebfactory[lev]));
        bcoeff[lev][2] = std::move(bc2_new);
        bcoeff[lev][2]->setVal(0.0);
    }

	// Molecular viscosity
	std::unique_ptr<MultiFab> eta_new(new MultiFab(grids[lev], dmap[lev], 1, nghost));
	eta_new->setVal(0.);
	eta_new->copy(*eta[lev], 0, 0, 1, 0, nghost);
	eta[lev] = std::move(eta_new);

	// Gas velocity
	std::unique_ptr<MultiFab> vel_new(new MultiFab(grids[lev], dmap[lev], vel[lev]->nComp(), nghost,
                                                   MFInfo(), *ebfactory[lev]));
	vel_new->setVal(0.);
	vel_new->copy(*vel[lev], 0, 0, vel[lev]->nComp(), 0, nghost);
	vel[lev] = std::move(vel_new);

	// Old gas velocity
    std::unique_ptr<MultiFab> vel_o_new(new MultiFab(grids[lev], dmap[lev], vel_o[lev]->nComp(), nghost,
                                                     MFInfo(), *ebfactory[lev]));
	vel_o_new->setVal(0.);
	vel_o_new->copy(*vel_o[lev], 0, 0, vel_o[lev]->nComp(), 0, nghost);
	vel_o[lev] = std::move(vel_o_new);

	// Pressure gradients
	std::unique_ptr<MultiFab> gp_new(new MultiFab(grids[lev], dmap[lev], 3, nghost));
    gp_new->setVal(0.);
	gp_new->copy(*gp[lev], 0, 0, 1, 0, nghost);
	gp[lev] = std::move(gp_new);

	// Pressure gradients
	std::unique_ptr<MultiFab> gp0_new(new MultiFab(grids[lev], dmap[lev], 3, nghost));
    gp0_new->setVal(0.);
	gp0_new->copy(*gp0[lev], 0, 0, 1, 0, nghost);
	gp0[lev] = std::move(gp0_new);

	// Strain-rate magnitude
	std::unique_ptr<MultiFab> strainrate_new(new MultiFab(grids[lev], dmap[lev], 1, nghost));
	strainrate[lev] = std::move(strainrate_new);
	strainrate[lev]->setVal(0.);

	// Vorticity
	std::unique_ptr<MultiFab> vort_new(new MultiFab(grids[lev], dmap[lev], 1, nghost));
	vort[lev] = std::move(vort_new);
	vort[lev]->setVal(0.);

	/****************************************************************************
    * Face-based Arrays                                                        *
    ****************************************************************************/

    BoxArray x_ba = grids[lev];
    x_ba = x_ba.surroundingNodes(0);

	// MAC velocity
	std::unique_ptr<MultiFab> u_mac_new(new MultiFab(x_ba, dmap[lev], 1, nghost, 
                                                     MFInfo(), *ebfactory[lev]));
	m_u_mac[lev] = std::move(u_mac_new);
	m_u_mac[lev]->setVal(0.0);

    BoxArray y_ba = grids[lev];
    y_ba = y_ba.surroundingNodes(1);

    // MAC velocity
    std::unique_ptr<MultiFab> v_mac_new(new MultiFab(y_ba, dmap[lev], 1, nghost, 
                                                     MFInfo(), *ebfactory[lev]));
    m_v_mac[lev] = std::move(v_mac_new);
    m_v_mac[lev] -> setVal(0.0);

    // Diffusion coefficient on y-faces
    std::unique_ptr<MultiFab> bc1_new(new MultiFab(y_ba, dmap[lev], 1, nghost, 
                                                   MFInfo(), *ebfactory[lev]));
    BoxArray z_ba = grids[lev];
    z_ba = z_ba.surroundingNodes(2);

    // MAC velocity
    std::unique_ptr<MultiFab> w_mac_new(new MultiFab(z_ba, dmap[lev], 1, nghost, 
                                                     MFInfo(), *ebfactory[lev]));
    m_w_mac[lev] = std::move(w_mac_new);
    m_w_mac[lev] -> setVal(0.0);

    // Diffusion coefficient on z-faces
    std::unique_ptr<MultiFab> bc2_new(new MultiFab(z_ba, dmap[lev], 1, nghost,
                                                   MFInfo(), *ebfactory[lev]));
    bcoeff[lev][2] = std::move(bc2_new);
    bcoeff[lev][2] -> setVal(0.0);

	//****************************************************************************

    // Slopes in x-direction
    std::unique_ptr<MultiFab> xslopes_new(new MultiFab(grids[lev], dmap[lev], xslopes[lev]->nComp(), nghost));
    xslopes[lev] = std::move(xslopes_new);
    xslopes[lev] -> setVal(0.);

    // Slopes in y-direction
    std::unique_ptr<MultiFab> yslopes_new(new MultiFab(grids[lev], dmap[lev], yslopes[lev]->nComp(), nghost));
    yslopes[lev] = std::move(yslopes_new);
    yslopes[lev] -> setVal(0.);

    // Slopes in z-direction
    std::unique_ptr<MultiFab> zslopes_new(new MultiFab(grids[lev], dmap[lev], zslopes[lev]->nComp(), nghost));
    zslopes[lev] = std::move(zslopes_new);
    zslopes[lev] -> setVal(0.);

	// ********************************************************************************
	// Make sure we fill the ghost cells as appropriate -- this is copied from init_fluid
	// ********************************************************************************

	FillScalarBC(lev, *ro[lev]);
	FillScalarBC(lev, *eta[lev]);

    if (!nodal_pressure) FillScalarBC(lev, *p[lev]);
}

// Resize all arrays when instance of incflo class is constructed.
// This is only done at the very start of the simulation. 
void incflo::ResizeArrays()
{
	p.resize(max_level + 1);
	p0.resize(max_level + 1);

	ro.resize(max_level + 1);

	phi.resize(max_level + 1);
	divu.resize(max_level + 1);

	// Current (vel) and old (vel_o) velocities
	vel.resize(max_level + 1);
	vel_o.resize(max_level + 1);

	// Pressure gradients
	gp.resize(max_level + 1);
	gp0.resize(max_level + 1);

    // Derived quantities: viscosity, strainrate, vorticity
	eta.resize(max_level + 1);
    strainrate.resize(max_level + 1);
	vort.resize(max_level + 1);

	// MAC velocities used for defining convective term
	m_u_mac.resize(max_level + 1);
	m_v_mac.resize(max_level + 1);
	m_w_mac.resize(max_level + 1);

    // Slopes used for upwinding convective terms
	xslopes.resize(max_level + 1);
	yslopes.resize(max_level + 1);
	zslopes.resize(max_level + 1);

    // Coefficients for elliptic solves
	bcoeff.resize(max_level + 1);
	for(int lev = 0; lev < max_level + 1; ++lev)
		bcoeff[lev].resize(3);

    // BCs
	bc_ilo.resize(max_level + 1);
	bc_ihi.resize(max_level + 1);
	bc_jlo.resize(max_level + 1);
	bc_jhi.resize(max_level + 1);
	bc_klo.resize(max_level + 1);
	bc_khi.resize(max_level + 1);

	// EB factory
	ebfactory.resize(max_level + 1);
}

void incflo::MakeBCArrays()
{
    for(int lev = 0; lev <= max_level; lev++)
    {
        // Define and allocate the integer MultiFab that is the outside adjacent cells of the
        // problem domain.
        Box domainx(geom[lev].Domain());
        domainx.grow(1, nghost);
        domainx.grow(2, nghost);
        Box box_ilo = amrex::adjCellLo(domainx, 0, 1);
        Box box_ihi = amrex::adjCellHi(domainx, 0, 1);

        Box domainy(geom[lev].Domain());
        domainy.grow(0, nghost);
        domainy.grow(2, nghost);
        Box box_jlo = amrex::adjCellLo(domainy, 1, 1);
        Box box_jhi = amrex::adjCellHi(domainy, 1, 1);

        Box domainz(geom[lev].Domain());
        domainz.grow(0, nghost);
        domainz.grow(1, nghost);
        Box box_klo = amrex::adjCellLo(domainz, 2, 1);
        Box box_khi = amrex::adjCellHi(domainz, 2, 1);

        // Note that each of these is a single IArrayBox so every process has a copy of them
        bc_ilo[lev].reset(new IArrayBox(box_ilo, 2));
        bc_ihi[lev].reset(new IArrayBox(box_ihi, 2));
        bc_jlo[lev].reset(new IArrayBox(box_jlo, 2));
        bc_jhi[lev].reset(new IArrayBox(box_jhi, 2));
        bc_klo[lev].reset(new IArrayBox(box_klo, 2));
        bc_khi[lev].reset(new IArrayBox(box_khi, 2));
    }
}

