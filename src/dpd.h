/* Header file for DPD Monte Carlo simulation program */

// Inline substitutions

#define P      sys.stats[0]
#define Etot   sys.stats[1]
#define RE2    sys.stats[2]
#define RE2x   sys.stats[3]
#define RE2y   sys.stats[4]
#define RE2z   sys.stats[5]
#define RG2    sys.stats[6]
#define RG2x   sys.stats[7]
#define RG2y   sys.stats[8]
#define RG2z   sys.stats[9]
#define BL     sys.stats[10]

/* Structure defintions */

typedef struct vector_type {
  double x, y, z;
} Vector;

typedef struct ivector_type {
  int x, y, z;
} Ivector;

typedef struct particle_type {
  int ll;
  double E, Eo;
  Vector r, ro;
} Particle;

typedef struct stats_type {
  char *name;
  int num;
  double now, sum, sumsq, err;
} Stats;

typedef struct monitor_type {
  double *energy, *re2, *rex, *rey, *rez,
    *rg2, *rgx, *rgy, *rgz, *cmx, *cmy, *cmz, *bond_length;
} Monitor;

typedef struct parameter_type {
  int
    bond_break,
    calc_list,
    freq_monitor,
    freq_sample,
    iseed,
    ***hoc,
    ***hoc_copy,
    monitor_step,
    n_steps,
    n_accept_dpd,
    n_accept_mon,
    n_attempt_dpd,
    n_attempt_mon,
    n_dpd,
    n_layers,
    n_mon,
    n_stats,
    n_wall,
    wall_overlap;

  double
    a_mm,
    a_ms,
    a_ss,
    a_sw,
    density_s,
    density_w,
    dr_max_dpd,
    dr_max_mon,
    energy,
    k_fene,
    mc_ratio,
    pol_init_bl,
    pol_init_z,
    r_c,
    r_eq,
    r_max,
    r_wall,
    r_0,
    temp,
    volume,
    wall_max_z,
    wall_min_z;

  Ivector n_cell;
  Vector length, r_cell;
  Stats *stats;
  Monitor mon;
} System;


/* Global variables */

extern Particle *part_dpd;
extern Particle *part_mon;
extern System sys;


/* Global functions */

extern int     accept_move(void);
extern void    calc_bond_length(void);
extern void    calc_energy_brute(void);
extern double  calc_energy_dpd(int i);
extern double  calc_energy_mon(int i);
extern void    calc_cm(void);
extern void    calc_pressure(void);
extern void    calc_re(void);
extern void    calc_rg(void);
extern void    check_bond(int i);
extern int     check_cell(Vector, Vector);
extern void    check_wall(Vector);
extern double  energy_c(Vector);
extern double  energy_fene(int i, int j);;
extern void    initialize(void);
extern void    init_param(void);
extern void    init_part(void);
extern void    init_stats(void);
extern void    init_wall(void);
extern void    input(void);
extern int     mod(int, int);
extern void    monitor_mem(void);
extern void    monte_carlo(void);
extern void    new_list(void);
extern void    periodic_bc_dr(Vector *);
extern void    periodic_bc_r(Vector *);
extern void    print_stats(void);
extern double  ran3(void);
extern void    random_move_dpd(int i);
extern void    random_move_mon(int i);
extern void    sample(void);
extern double  total_energy(void);
extern void    write_log(void);
extern void    write_mon(void);
extern Vector  vdist(Vector, Vector);
extern double  vmag(Vector);

