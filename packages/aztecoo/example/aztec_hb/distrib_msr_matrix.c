/*@HEADER
// ************************************************************************
// 
//          Trilinos: An Object-Oriented Solver Framework
//              Copyright (2002) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//   
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//   
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
// 
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
// 
// ************************************************************************
//@HEADER
*/

#include <stdlib.h>
#include <stdio.h>
#include "az_aztec.h"
#include "prototypes.h"
/*#define DEBUG */
void distrib_msr_matrix(int *proc_config, int N_global,
	      int *n_nonzeros, int *N_update, int **update,
	      double **val, int **bindx,
	      double **x, double **b, double **xexact)

{
  int i, n_entries, N_columns, n_global_nonzeros;
  int ii, j, row, have_xexact = 0 ;
  int kk = 0;
  int max_ii = 0, max_jj = 0;
  int ione = 1;
  double value;
  double *cnt;
  int *pntr, *bindx1, *pntr1;
  double *val1, *b1, *x1, *xexact1;
#ifdef DEBUG
  printf("Processor %d of %d entering distrib_matrix.\n",
	 proc_config[AZ_node],proc_config[AZ_N_procs]) ;
#endif

  /*************** Distribute global matrix to all processors ************/

  if(proc_config[AZ_node] == 0)
    {
      if ((*xexact) != NULL) have_xexact = 1;
#ifdef DEBUG
      printf("Broadcasting exact solution\n");
#endif
    }

  if(proc_config[AZ_N_procs]  > 1)
    { 

      AZ_broadcast((char *) &N_global,        sizeof(int), proc_config, AZ_PACK);
      AZ_broadcast((char *) n_nonzeros, sizeof(int), proc_config, AZ_PACK);
      AZ_broadcast((char *) &have_xexact, sizeof(int), proc_config, AZ_PACK);
      AZ_broadcast(NULL, 0, proc_config, AZ_SEND);

      if(proc_config[AZ_node] != 0)
	{
	  (*bindx) = (int   *) calloc(*n_nonzeros+1,sizeof(int)) ;
	  (*val) = (double *) calloc(*n_nonzeros+1,sizeof(double)) ;
	}

      AZ_broadcast((char *) (*bindx), sizeof(int)   *(*n_nonzeros+1), 
		   proc_config, AZ_PACK);
      AZ_broadcast(NULL, 0, proc_config, AZ_SEND);
      AZ_broadcast((char *) (*val),  sizeof(double)*(*n_nonzeros+1), 
		   proc_config, AZ_PACK);
      AZ_broadcast(NULL, 0, proc_config, AZ_SEND);

#ifdef DEBUG
      printf("Processor %d of %d done with matrix broadcast.\n",
	     proc_config[AZ_node],proc_config[AZ_N_procs]) ;
#endif
 
      /* Set rhs and initialize guess */
      if(proc_config[AZ_node] != 0)
	{
	  (*b) = (double *) calloc(N_global,sizeof(double)) ;
	  (*x) = (double *) calloc(N_global,sizeof(double)) ;
	  if (have_xexact)
	  (*xexact) =   (double *) calloc(N_global,sizeof(double)) ;
	}

      AZ_broadcast((char *) (*x), sizeof(double)*(N_global), proc_config, AZ_PACK);
      AZ_broadcast((char *) (*b), sizeof(double)*(N_global), proc_config, AZ_PACK);
      if (have_xexact)
	AZ_broadcast((char *) 
		     (*xexact), sizeof(double)*(N_global), proc_config, AZ_PACK);
      AZ_broadcast(NULL, 0, proc_config, AZ_SEND);
#ifdef DEBUG
      printf("Processor %d of %d done with rhs/guess broadcast.\n",
	     proc_config[AZ_node],proc_config[AZ_N_procs]) ;
#endif

    }

  /********************** Generate update map  *************************/

#ifdef AZ_FUNKY_READ_UPDATE
  AZ_read_update_funky(N_update, update, proc_config, N_global,
           1, AZ_linear) ;
#else
  AZ_read_update(N_update, update, proc_config, N_global,
           1, AZ_linear) ;
#endif
  
  printf("Processor %d of %d has %d rows of %d total rows.\n",
	 proc_config[AZ_node],proc_config[AZ_N_procs],*N_update,N_global) ;

  /*************** Construct local matrix from global matrix ************/

  /* The local matrix is a copy of the rows assigned to this processor.  
     It is stored in MSR format and still has global indices (AZ_transform
     will complete conversion to local indices.
  */

  if(proc_config[AZ_N_procs]  > 1)
    { 
      n_global_nonzeros = *n_nonzeros;

      *n_nonzeros = *N_update;
      
      for (i=0; i<*N_update; i++)
	*n_nonzeros += (*bindx)[(*update)[i]+1] - (*bindx)[(*update)[i]];

      printf("Processor %d of %d has %d nonzeros of %d total nonzeros.\n",
	     proc_config[AZ_node],proc_config[AZ_N_procs],
	     *n_nonzeros,n_global_nonzeros) ;

#ifdef DEBUG
      { double sum1 = 0.0;
      for (i=0;i<N_global; i++) sum1 += (*b)[i];

      printf("Processor %d of %d has sum of b = %12.4g.\n",
	     proc_config[AZ_node],proc_config[AZ_N_procs],sum1) ;
      }
#endif /* DEBUG */

      /* Allocate memory for local matrix */

      bindx1 = (int   *) calloc(*n_nonzeros+1,sizeof(int)) ;
      val1 = (double *) calloc(*n_nonzeros+1,sizeof(double)) ;
      b1 =   (double *) calloc(*N_update,sizeof(double)) ;
      x1 =   (double *) calloc(*N_update,sizeof(double)) ;
      if (have_xexact)
      xexact1 =   (double *) calloc(*N_update,sizeof(double)) ;
     
      bindx1[0] = *N_update+1;
      
      for (i=0; i<*N_update; i++)
	{
	  row = (*update)[i];
	  b1[i] = (*b)[row];
	  x1[i] = (*x)[row];
	  if (have_xexact) xexact1[i] = (*xexact)[row];
	  val1[i] = (*val)[row];
	  bindx1[i+1] = bindx1[i];

#ifdef DEBUG	  
	  printf("Proc %d of %d: Global row = %d: Local row = %d: 
                  b = %12.4g: x = %12.4g: bindx = %d: val = %12.4g \n",
		 proc_config[AZ_node],proc_config[AZ_N_procs], 
		 row, i, b1[i], x1[i], bindx1[i], val1[i]) ;
#endif

	  for (j = (*bindx)[row]; j < (*bindx)[row+1]; j++)
	    {
	      val1[  bindx1 [i+1] ] = (*val)[j];
	      bindx1[bindx1 [i+1] ] = (*bindx)[j];
	      bindx1[i+1] ++;
	    }
	}

#ifdef DEBUG
      printf("Processor %d of %d done with extracting local operators.\n",
	     proc_config[AZ_node],proc_config[AZ_N_procs]) ;

      if (have_xexact)
	{
	  printf(
     "The residual using MSR format and exact solution on processor %d is %12.4g\n",
	      proc_config[AZ_node],
	      smsrres (*N_update, N_global, val1, bindx1, xexact1, (*xexact), b1));
	}
  
#endif
      /* Release memory for global matrix, rhs and solution */
      
      free ((void *) (*val));
      free ((void *) (*bindx));
      free ((void *) (*b));
      free ((void *) (*x));
      if (have_xexact) free((void *) *xexact);

      /* Return local matrix through same pointers. */
      
      *val = val1;
      *bindx = bindx1;
      *b = b1;
      *x = x1;
      if (have_xexact) *xexact = xexact1;

    }
#ifdef DEBUG
      if (have_xexact && proc_config[AZ_N_procs]  == 1)
	{
	  printf(
     "The residual using MSR format and exact solution on processor %d is %12.4g\n",
	      proc_config[AZ_node],
	      smsrres (*N_update, N_global, (*val), (*bindx), 
		       (*xexact), (*xexact), (*b)));
	}

  
  printf("Processor %d of %d leaving distrib_matrix.\n",
	 proc_config[AZ_node],proc_config[AZ_N_procs]) ;
#endif  
  /* end distrib_matrix */
}
