#define HERMES_REPORT_ALL
#define HERMES_REPORT_FILE "application.log"
#include "../definitions.h"

using namespace RefinementSelectors;

// This test makes sure that example 21-newton-timedep-ns works correctly.

const bool STOKES = false;                        // For application of Stokes flow (creeping flow).
#define PRESSURE_IN_L2                            // If this is defined, the pressure is approximated using
                                                  // discontinuous L2 elements (making the velocity discreetely
                                                  // divergence-free, more accurate than using a continuous
                                                  // pressure approximation). Otherwise the standard continuous
                                                  // elements are used. The results are striking - check the
                                                  // tutorial for comparisons.
const bool NEWTON = true;                         // If NEWTON == true then the Newton's iteration is performed.
                                                  // in every time step. Otherwise the convective term is linearized
                                                  // using the velocities from the previous time step.
const int P_INIT_VEL = 2;                         // Initial polynomial degree for velocity components.
const int P_INIT_PRESSURE = 1;                    // Initial polynomial degree for pressure.
                                                  // Note: P_INIT_VEL should always be greater than
                                                  // P_INIT_PRESSURE because of the inf-sup condition.
const double RE = 200.0;                          // Reynolds number.
const double VEL_INLET = 1.0;                     // Inlet velocity (reached after STARTUP_TIME).
const double STARTUP_TIME = 1.0;                  // During this time, inlet velocity increases gradually
                                                  // from 0 to VEL_INLET, then it stays constant.
const double TAU = 0.1;                           // Time step.
const double T_FINAL = 0.21;                      // Time interval length.
const double NEWTON_TOL = 1e-3;                   // Stopping criterion for the Newton's method.
const int NEWTON_MAX_ITER = 10;                   // Maximum allowed number of Newton iterations.
const double H = 5;                               // Domain height (necessary to define the parabolic
                                                  // velocity profile at inlet).
MatrixSolverType matrix_solver = SOLVER_UMFPACK;  // Possibilities: SOLVER_AMESOS, SOLVER_AZTECOO, SOLVER_MUMPS,
                                                  // SOLVER_PETSC, SOLVER_SUPERLU, SOLVER_UMFPACK.

// Boundary markers.
const std::string BDY_BOTTOM = "b1";
const std::string BDY_RIGHT = "b2";
const std::string BDY_TOP = "b3";
const std::string BDY_LEFT = "b4";
const std::string BDY_OBSTACLE = "b5";

// Current time (used in weak forms).
double current_time = 0;

int main(int argc, char* argv[])
{
  Hermes2D hermes_2D;

  // Load the mesh.
  Mesh mesh;
  H2DReader mloader;
  mloader.load("../domain.mesh", &mesh);

  // Initial mesh refinements.
  mesh.refine_all_elements();
  mesh.refine_towards_boundary(BDY_OBSTACLE, 4, false);
  mesh.refine_towards_boundary(BDY_TOP, 4, true);     // '4' is the number of levels,
  mesh.refine_towards_boundary(BDY_BOTTOM, 4, true);  // 'true' stands for anisotropic refinements.

  // Initialize boundary conditions.
  EssentialBCNonConst bc_left_vel_x(BDY_LEFT, VEL_INLET, H, STARTUP_TIME);
  DefaultEssentialBCConst bc_other_vel_x(Hermes::vector<std::string>(BDY_BOTTOM, BDY_TOP, BDY_OBSTACLE), 0.0);
  EssentialBCs bcs_vel_x(Hermes::vector<EssentialBoundaryCondition *>(&bc_left_vel_x, &bc_other_vel_x));
  DefaultEssentialBCConst bc_vel_y(Hermes::vector<std::string>(BDY_LEFT, BDY_BOTTOM, BDY_TOP, BDY_OBSTACLE), 0.0);
  EssentialBCs bcs_vel_y(&bc_vel_y);
  EssentialBCs bcs_pressure;

  // Spaces for velocity components and pressure.
  H1Space xvel_space(&mesh, &bcs_vel_x, P_INIT_VEL);
  H1Space yvel_space(&mesh, &bcs_vel_y, P_INIT_VEL);
#ifdef PRESSURE_IN_L2
  L2Space p_space(&mesh, &bcs_pressure, P_INIT_PRESSURE);
#else
  H1Space p_space(&mesh, &bcs_pressure, P_INIT_PRESSURE);
#endif

  // Calculate and report the number of degrees of freedom.
  int ndof = Space::get_num_dofs(Hermes::vector<Space *>(&xvel_space, &yvel_space, &p_space));
  info("ndof = %d.", ndof);

  // Define projection norms.
  ProjNormType vel_proj_norm = HERMES_H1_NORM;
#ifdef PRESSURE_IN_L2
  ProjNormType p_proj_norm = HERMES_L2_NORM;
#else
  ProjNormType p_proj_norm = HERMES_H1_NORM;
#endif

  // Solutions for the Newton's iteration and time stepping.
  info("Setting initial conditions.");
  Solution xvel_prev_time, yvel_prev_time, p_prev_time;
  xvel_prev_time.set_zero(&mesh);
  yvel_prev_time.set_zero(&mesh);
  p_prev_time.set_zero(&mesh);

  // Initialize weak formulation.
  WeakForm* wf;
  if (NEWTON)
    wf = new WeakFormNSNewton(STOKES, RE, TAU, &xvel_prev_time, &yvel_prev_time);
  else
    wf = new WeakFormNSSimpleLinearization(STOKES, RE, TAU, &xvel_prev_time, &yvel_prev_time);

  // Initialize the FE problem.
  DiscreteProblem dp(wf, Hermes::vector<Space *>(&xvel_space, &yvel_space, &p_space));

  // Set up the solver, matrix, and rhs according to the solver selection.
  SparseMatrix* matrix = create_matrix(matrix_solver);
  Vector* rhs = create_vector(matrix_solver);
  Solver* solver = create_linear_solver(matrix_solver, matrix, rhs);

  // Project the initial condition on the FE space to obtain initial
  // coefficient vector for the Newton's method.
  scalar* coeff_vec = new scalar[Space::get_num_dofs(Hermes::vector<Space *>(&xvel_space, &yvel_space, &p_space))];
  if (NEWTON) {
    info("Projecting initial condition to obtain initial vector for the Newton's method.");
    OGProjection::project_global(Hermes::vector<Space *>(&xvel_space, &yvel_space, &p_space),
                   Hermes::vector<MeshFunction *>(&xvel_prev_time, &yvel_prev_time, &p_prev_time),
                   coeff_vec, matrix_solver,
                   Hermes::vector<ProjNormType>(vel_proj_norm, vel_proj_norm, p_proj_norm));
  }

  // Time-stepping loop:
  char title[100];
  int num_time_steps = T_FINAL / TAU;
  for (int ts = 1; ts <= num_time_steps; ts++)
  {
    current_time += TAU;
    info("---- Time step %d, time = %g:", ts, current_time);

    // Update time-dependent essential BCs.
    if (current_time <= STARTUP_TIME) {
      info("Updating time-dependent essential BC.");
      Space::update_essential_bc_values(Hermes::vector<Space *>(&xvel_space, &yvel_space, &p_space), current_time);
    }

    if (NEWTON)
    {
      // Perform Newton's iteration.
      info("Solving nonlinear problem:");
      bool verbose = true;
      bool jacobian_changed = true;
      if (!hermes_2D.solve_newton(coeff_vec, &dp, solver, matrix, rhs, jacobian_changed,
          NEWTON_TOL, NEWTON_MAX_ITER, verbose)) error("Newton's iteration failed.");

      // Update previous time level solutions.
      Solution::vector_to_solutions(coeff_vec, Hermes::vector<Space *>(&xvel_space, &yvel_space, &p_space),
                                    Hermes::vector<Solution *>(&xvel_prev_time, &yvel_prev_time, &p_prev_time));
    }
    else {
      // Linear solve.
      info("Assembling and solving linear problem.");
      dp.assemble(matrix, rhs, false);
      if(solver->solve())
        Solution::vector_to_solutions(solver->get_solution(),
                  Hermes::vector<Space *>(&xvel_space, &yvel_space, &p_space),
                  Hermes::vector<Solution *>(&xvel_prev_time, &yvel_prev_time, &p_prev_time));
      else
        error ("Matrix solver failed.\n");
    }
 }

  delete [] coeff_vec;
  delete matrix;
  delete rhs;
  delete solver;

  info("Coordinate (   0, 2.5) xvel value = %lf", xvel_prev_time.get_pt_value(0.0, 2.5));
  info("Coordinate (   5, 2.5) xvel value = %lf", xvel_prev_time.get_pt_value(5.0, 2.5));
  info("Coordinate ( 7.5, 2.5) xvel value = %lf", xvel_prev_time.get_pt_value(7.5, 2.5));
  info("Coordinate (  10, 2.5) xvel value = %lf", xvel_prev_time.get_pt_value(10.0, 2.5));
  info("Coordinate (12.5, 2.5) xvel value = %lf", xvel_prev_time.get_pt_value(12.5, 2.5));
  info("Coordinate (  15, 2.5) xvel value = %lf", xvel_prev_time.get_pt_value(15.0, 2.5));

  info("Coordinate (   0, 2.5) yvel value = %lf", yvel_prev_time.get_pt_value(0.0, 2.5));
  info("Coordinate (   5, 2.5) yvel value = %lf", yvel_prev_time.get_pt_value(5.0, 2.5));
  info("Coordinate ( 7.5, 2.5) yvel value = %lf", yvel_prev_time.get_pt_value(7.5, 2.5));
  info("Coordinate (  10, 2.5) yvel value = %lf", yvel_prev_time.get_pt_value(10.0, 2.5));
  info("Coordinate (12.5, 2.5) yvel value = %lf", yvel_prev_time.get_pt_value(12.5, 2.5));
  info("Coordinate (  15, 2.5) yvel value = %lf", yvel_prev_time.get_pt_value(15.0, 2.5));

  int success = 1;
  double eps = 1e-5;
  if (fabs(xvel_prev_time.get_pt_value(0.0, 2.5) - 0.200000) > eps) {
    printf("Coordinate (   0, 2.5) xvel value is %g\n", xvel_prev_time.get_pt_value(0.0, 2.5));
    success = 0;
  }
  if (fabs(xvel_prev_time.get_pt_value(5, 2.5) - 0.130866) > eps) {
    printf("Coordinate (   5, 2.5) xvel value is %g\n", xvel_prev_time.get_pt_value(5, 2.5));
    success = 0;
  }
  if (fabs(xvel_prev_time.get_pt_value(7.5, 2.5) - 0.134637) > eps) {
    printf("Coordinate ( 7.5, 2.5) xvel value is %g\n", xvel_prev_time.get_pt_value(7.5, 2.5));
    success = 0;
  }
  if (fabs(xvel_prev_time.get_pt_value(10, 2.5) - 0.134801) > eps) {
    printf("Coordinate (  10, 2.5) xvel value is %g\n", xvel_prev_time.get_pt_value(10, 2.5));
    success = 0;
  }
  if (fabs(xvel_prev_time.get_pt_value(12.5, 2.5) - 0.134826) > eps) {
    printf("Coordinate (12.5, 2.5) xvel value is %g\n", xvel_prev_time.get_pt_value(12.5, 2.5));
    success = 0;
  }
  if (fabs(xvel_prev_time.get_pt_value(15, 2.5) - 0.134840) > eps) {
    printf("Coordinate (  15, 2.5) xvel value is %g\n", xvel_prev_time.get_pt_value(15, 2.5));
    success = 0;
  }

  if (fabs(yvel_prev_time.get_pt_value(0.0, 2.5) - 0.000000) > eps) {
    printf("Coordinate (   0, 2.5) yvel value is %g\n", yvel_prev_time.get_pt_value(0.0, 2.5));
    success = 0;
  }
  if (fabs(yvel_prev_time.get_pt_value(5, 2.5) - 0.000584) > eps) {
    printf("Coordinate (   5, 2.5) yvel value is %g\n", yvel_prev_time.get_pt_value(5, 2.5));
    success = 0;
  }
  if (fabs(yvel_prev_time.get_pt_value(7.5, 2.5) - 0.000101) > eps) {
    printf("Coordinate ( 7.5, 2.5) yvel value is %g\n", yvel_prev_time.get_pt_value(7.5, 2.5));
    success = 0;
  }
  if (fabs(yvel_prev_time.get_pt_value(10, 2.5) - 0.000029) > eps) {
    printf("Coordinate (  10, 2.5) yvel value is %g\n", yvel_prev_time.get_pt_value(10, 2.5));
    success = 0;
  }
  if (fabs(yvel_prev_time.get_pt_value(12.5, 2.5) - 0.000013) > eps) {
    printf("Coordinate (12.5, 2.5) yvel value is %g\n", yvel_prev_time.get_pt_value(12.5, 2.5));
    success = 0;
  }
  if (fabs(yvel_prev_time.get_pt_value(15, 2.5) - 0.000009) > eps) {
    printf("Coordinate (  15, 2.5) yvel value is %g\n", yvel_prev_time.get_pt_value(15, 2.5));
    success = 0;
  }

  if (success == 1) {
    printf("Success!\n");
    return ERR_SUCCESS;
  }
  else {
    printf("Failure!\n");
    return ERR_FAILURE;
  }
}
