/* init.c */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "dpd.h"

void initialize(void) {
  input();
  init_param();
  init_part();
  init_pore();
  init_wall();
  init_monitor();
  init_stats();
  write_log();
}

void init_param(void) {
  int i, j, fact;

  sys.monitor_step = 0;
  sys.n_accept_mon = 0;
  sys.n_accept_solvent = 0;
  sys.n_attempt_mon = 0;
  sys.n_attempt_solvent = 0;

  // Wall particle spacing
  sys.r_wall = pow(sys.density_w, -1.0/3.0);

  // Pore particle spacing
  sys.n_pore_1d.x = (int) (2*sys.pore_radius / sys.r_wall + 0.5) + 1;
  sys.r_pore = 2*sys.pore_radius / (sys.n_pore_1d.x-1);
  sys.n_pore_1d.y = sys.n_pore_1d.x;

  // Pore bounds
  sys.pore_max.x = sys.length.x/2 + sys.pore_radius;
  sys.pore_min.x = sys.length.x/2 - sys.pore_radius;
  sys.pore_max.y = sys.length.y/2 + sys.pore_radius;
  sys.pore_min.y = sys.length.y/2 - sys.pore_radius;
  sys.pore_max.z = sys.length.z/2;
  sys.n_pore_1d.z = (int) (sys.pore_max.z / sys.r_wall) + 1;
  sys.pore_min.z = sys.pore_max.z - (sys.n_pore_1d.z-1)*sys.r_wall;

  // Wall bounds
  sys.n_wall_1d.z = sys.n_layers + 1;
  sys.wall_max.z = sys.length.z/2;
  sys.wall_min.z = sys.length.z/2 - sys.n_layers*sys.r_wall;
  sys.n_wall_1d.x = (int) (sys.pore_min.x / sys.r_wall);
  sys.wall_min.x = sys.pore_min.x - (sys.n_wall_1d.x)*sys.r_wall;
  sys.wall_max.x = sys.pore_max.x + (sys.n_wall_1d.x)*sys.r_wall;
  sys.n_wall_1d.x = 2*sys.n_wall_1d.x +  sys.n_pore_1d.x;
  sys.n_wall_1d.y = (int) (sys.pore_min.y / sys.r_wall);
  sys.wall_min.y = sys.pore_min.y - (sys.n_wall_1d.y)*sys.r_wall;
  sys.wall_max.y = sys.pore_max.y + (sys.n_wall_1d.y)*sys.r_wall;
  sys.n_wall_1d.y = 2*sys.n_wall_1d.y +  sys.n_pore_1d.y;

  // Adjust the system length to keep a consistent wall density
  sys.length.x += sys.r_wall - (sys.wall_min.x+sys.length.x) + sys.wall_max.x;
  sys.length.y += sys.r_wall - (sys.wall_min.y+sys.length.y) + sys.wall_max.y;

  // Number of pore particles
  sys.n_pore = 2 * (sys.n_pore_1d.x + sys.n_pore_1d.y - 2);
  sys.n_pore *= sys.n_pore_1d.z;

  // Number of wall particles
  sys.n_wall = sys.n_wall_1d.x * sys.n_wall_1d.y;
  sys.n_wall -= sys.n_pore_1d.x * sys.n_pore_1d.y;
  sys.n_wall *= sys.n_wall_1d.z;
  // Wall particles surrounding the pore (as part of layers)
  sys.n_wall += (sys.n_pore_1d.z-sys.n_layers-1) * (2*sys.n_layers)
    * (sys.n_pore_1d.x + sys.n_pore_1d.y + 2*sys.n_layers);

  // Wall volume
  sys.wall_volume = sys.length.x * sys.length.y;
  sys.wall_volume -= (sys.pore_max.x - sys.pore_min.x)*(sys.pore_max.y - sys.pore_min.y);
  sys.wall_volume *= sys.n_layers * sys.r_wall;

  // Pore volume
  sys.pore_volume = sys.pore_max.x - sys.pore_min.x + 2*sys.n_layers*sys.r_wall;
  sys.pore_volume *= sys.pore_max.y - sys.pore_min.y + 2*sys.n_layers*sys.r_wall;
  sys.pore_volume *= sys.pore_max.z - sys.pore_min.z;

  // Solvent volume and number of solvent particles
  sys.volume = sys.length.x * sys.length.y * sys.length.z;
  sys.volume -= sys.wall_volume + sys.pore_volume;
  sys.n_solvent = (int) (sys.density_s * sys.volume);

  // Total dpd particles
  sys.n_dpd = sys.n_solvent + sys.n_pore + sys.n_wall;
  part_dpd = (Particle *) calloc(sys.n_dpd, sizeof(Particle));

  // Monomer-monomer bonds
  sys.r_max = 2.0;
  sys.r_eq = 0.7;
  sys.r_0 = sys.r_max - sys.r_eq;
  sys.k_fene = 40;

  if (sys.calc_list) {
    // Determine cell size and the number of cells
    fact = (int) (sys.length.x / sys.r_c + 1e-9);
    sys.r_cell.x = (double) (sys.length.x / fact);
    sys.n_cell_1d.x = (int) (sys.length.x / sys.r_cell.x + 1e-9);

    fact = (int) (sys.length.y / sys.r_c + 1e-9);
    sys.r_cell.y = (double) (sys.length.y / fact);
    sys.n_cell_1d.y = (int) (sys.length.y / sys.r_cell.y + 1e-9);

    fact = (int) (sys.length.z / sys.r_c + 1e-9);
    sys.r_cell.z = (double) (sys.length.z / fact);
    sys.n_cell_1d.z = (int) (sys.length.z / sys.r_cell.z + 1e-9);

    // Allocate memory for the head of chain array
    sys.hoc = (int ***) malloc((sys.n_cell_1d.x+1)*sizeof(int **));
    sys.hoc_copy = (int ***) malloc((sys.n_cell_1d.x+1)*sizeof(int **));

    for (i = 0; i < sys.n_cell_1d.x; i++) {
      sys.hoc[i] = (int **) malloc((sys.n_cell_1d.y+1)*sizeof(int *));
      sys.hoc_copy[i] = (int **) malloc((sys.n_cell_1d.y+1)*sizeof(int *));

      for (j = 0; j < sys.n_cell_1d.y; j++) {
        sys.hoc[i][j] = (int *) malloc((sys.n_cell_1d.z+1)*sizeof(int));
        sys.hoc_copy[i][j] = (int *) malloc((sys.n_cell_1d.z+1)*sizeof(int));
      }
    }
  }
}

void init_part(void) {
  int i, inside_pore;
  double x, y, z, d;

  for (i = 0; i < sys.n_solvent; i++) {
    // Repeat until the particle is outside the wall and pore
    do {
      part_dpd[i].r.x = sys.length.x*ran3();
      part_dpd[i].r.y = sys.length.y*ran3();
      part_dpd[i].r.z = sys.length.z*ran3();
      part_dpd[i].ro.x = part_dpd[i].r.x;
      part_dpd[i].ro.y = part_dpd[i].r.y;
      part_dpd[i].ro.z = part_dpd[i].r.z;

      // Check for wall overlap
      check_wall(part_dpd[i].r);
      // Check for pore overlap
      inside_pore = check_pore(part_dpd[i].r);
    } while (sys.wall_overlap || sys.pore_overlap || inside_pore);
  }

  part_mon = (Particle *) calloc(sys.n_mon, sizeof(Particle));

  for (i = 0; i < sys.n_mon; i++) {
    part_mon[i].r.x = (sys.pore_max.x - sys.pore_radius) / 2;
    part_mon[i].r.y = (sys.pore_max.y - sys.pore_radius) / 2;
    part_mon[i].r.z = sys.pol_init_z + i*sys.pol_init_bl;

    periodic_bc_r(&part_mon[i].r);

    part_mon[i].ro.x = part_mon[i].r.x;
    part_mon[i].ro.y = part_mon[i].r.y;
    part_mon[i].ro.z = part_mon[i].r.z;
  }

  if (sys.calc_list) {
    new_list();

    for (i = 0; i < sys.n_solvent; i++) {
      part_dpd[i].E = calc_energy_dpd(i);
      part_dpd[i].Eo = part_dpd[i].E;
      // printf("part_dpd[%d].r = (%lf,%lf,%lf)\tpart_dpd[%d].E = %lf->%lf\n",i,part_dpd[i].r.x,part_dpd[i].r.y,part_dpd[i].r.z,i,part_dpd[i].Eo,part_dpd[i].E);
    }

    for (i = 0; i < sys.n_mon; i++) {
      part_mon[i].E = calc_energy_mon(i);
      part_mon[i].Eo = part_mon[i].E;
      printf("part_mon[%d].r = (%lf,%lf,%lf)\tpart_mon[%d].E = %lf->%lf\n",i,part_mon[i].r.x,part_mon[i].r.y,part_mon[i].r.z,i,part_mon[i].Eo,part_mon[i].E);
    }
  } else {
    calc_energy_brute();

    for (i = 0; i < sys.n_solvent; i++) {
      part_dpd[i].Eo = part_dpd[i].E;
    }
<<<<<<< HEAD
=======

>>>>>>> polymer
    for (i = 0; i < sys.n_mon; i++) {
      part_mon[i].Eo = part_mon[i].E;
    }
  }

  sys.energy = total_energy();
}

void init_pore(void) {
  int i, j, k, n;
  Vector r;

  n = sys.n_solvent + sys.n_wall;
  r.x = sys.pore_min.x;
  r.y = sys.pore_min.y;
  r.z = sys.pore_min.z;

  for (k = 0; k < sys.n_pore_1d.z; k++) {
    for (j = 0; j < sys.n_pore_1d.y; j++) {
      for (i = 0; i < sys.n_pore_1d.x; i++) {
        if (i == 0 || i == sys.n_pore_1d.x-1) {
          part_dpd[n].r = r;
          n++;
        } else if (j == 0 || j == sys.n_pore_1d.y-1) {
          part_dpd[n].r = r;
          n++;
        }
        r.x += sys.r_pore;
      }
      r.x = sys.pore_min.x;
      r.y += sys.r_pore;
    }
    r.y = sys.pore_min.y;
    r.z += sys.r_wall;
  }
}

void init_wall(void) {
  int i, j, k, n;
  double x, y, z;
  Vector r;

  r.x = sys.wall_min.x;
  r.y = sys.wall_min.y;
  r.z = sys.wall_min.z;
  n = sys.n_solvent;

  // Generate the wall in the xy plane
  for (k = 0; k < sys.n_wall_1d.z; k++) {
    for (j = 0; j < sys.n_wall_1d.y; j++) {
      for (i = 0; i < sys.n_wall_1d.x; i++) {

        if (r.x < sys.pore_min.x || r.x > sys.pore_max.x+1e-9) {
          part_dpd[n].r = r;
          n++;
        } else if (r.y < sys.pore_min.y || r.y > sys.pore_max.y+1e-9) {
          part_dpd[n].r = r;
          n++;
        }

        if (r.x >= sys.pore_min.x && r.x < sys.pore_max.x) {
          r.x += sys.r_pore;
        } else {
          r.x += sys.r_wall;
        }
      }

      r.x = sys.wall_min.x;

      if (r.y >= sys.pore_min.y && r.y < sys.pore_max.y) {
        r.y += sys.r_pore;
      } else {
        r.y += sys.r_wall;
      }
    }

    r.y = sys.wall_min.y;

    r.z += sys.r_wall;
  }

  // Add layers to the nanopore
  if (sys.n_layers > 0) {
    r.x = sys.pore_min.x - sys.n_layers*sys.r_wall;
    r.y = sys.pore_min.y - sys.n_layers*sys.r_wall;
    r.z = sys.pore_min.z;

    for (k = sys.n_wall_1d.z; k < sys.n_pore_1d.z; k++) {
      for (j = -sys.n_layers; j < sys.n_pore_1d.y+sys.n_layers; j++) {
        for (i = -sys.n_layers; i < sys.n_pore_1d.x+sys.n_layers; i++) {
          if (i < 0 || i > sys.n_pore_1d.x-1) {
            part_dpd[n].r = r;
            n++;
          } else if (j < 0 || j > sys.n_pore_1d.y-1) {
            part_dpd[n].r = r;
            n++;
          }

          if (i < 0 || i >= sys.n_pore_1d.x-1) {
            r.x += sys.r_wall;
          } else {
             r.x += sys.r_pore;
          }
        }

        r.x = sys.pore_min.x - sys.n_layers*sys.r_wall;

        if (j < 0 || j >= sys.n_pore_1d.y-1) {
          r.y += sys.r_wall;
        } else {
          r.y += sys.r_pore;
        }
      }

      r.y = sys.pore_min.y - sys.n_layers*sys.r_wall;

      r.z += sys.r_wall;
    }
  }
}

void init_monitor(void) {
  int nsize;

  nsize = (sys.n_steps / sys.freq_monitor) + 1;

  sys.mon.energy = (double *) calloc(nsize, sizeof(double));
  sys.mon.re2 = (double *) calloc(nsize, sizeof(double));
  sys.mon.rex = (double *) calloc(nsize, sizeof(double));
  sys.mon.rey = (double *) calloc(nsize, sizeof(double));
  sys.mon.rez = (double *) calloc(nsize, sizeof(double));
  sys.mon.rg2 = (double *) calloc(nsize, sizeof(double));
  sys.mon.rgx = (double *) calloc(nsize, sizeof(double));
  sys.mon.rgy = (double *) calloc(nsize, sizeof(double));
  sys.mon.rgz = (double *) calloc(nsize, sizeof(double));
  sys.mon.cmx = (double *) calloc(nsize, sizeof(double));
  sys.mon.cmy = (double *) calloc(nsize, sizeof(double));
  sys.mon.cmz = (double *) calloc(nsize, sizeof(double));
  sys.mon.bond_length = (double *) calloc(nsize, sizeof(double));
}

void init_stats(void) {
  sys.n_stats = 11;
  sys.stats = (Stats *) calloc(sys.n_stats, sizeof(Stats));

  sys.stats[0].name = "Pressure               ";
  sys.stats[1].name = "Energy                 ";
  sys.stats[2].name = "Re2                    ";
  sys.stats[3].name = "Re2x                   ";
  sys.stats[4].name = "Re2y                   ";
  sys.stats[5].name = "Re2z                   ";
  sys.stats[6].name = "Rg2                    ";
  sys.stats[7].name = "Rg2x                   ";
  sys.stats[8].name = "Rg2y                   ";
  sys.stats[9].name = "Rg2z                   ";
  sys.stats[10].name = "Bond_length            ";
}
