#ifndef _PETRA_RDP_CRS_MATRIX_H_
#define _PETRA_RDP_CRS_MATRIX_H_
//! Petra_RDP_CRS_Matrix: A class for constructing and using real-valued double-precision sparse compressed row matrices.

/*! The Petra_RDP_CRS_Matrix enable the piecewise construction and use of real-valued double-precision sparse matrices
    where matrix entries are intended for row access.

    At this time, the primary function provided by Petra_RDP_CRS_Matrix is matrix time vector and matrix 
    times multi-vector multiplication.  It is also possible to extract matrix rows from a constructed matrix.

<b>Constructing Petra_RDP_CRS_Matrix objects</b>

Constructing Petra_RDP_CRS_Matrix objects is a multi-step process.  The basic steps are as follows:
<ol>
  <li> Create Petra_RDP_CRS_Matrix instance, including storage,  via constructor.
  <li> Enter values via one or more Put or SumInto functions.
  <li> Complete construction via FillComplete call.
</ol>

Note that, even after a matrix is constructed, it is possible to update existing matrix entries.  It is \e not possible to
create new entries.

<b> Counting Floating Point Operations </b>

Each Petra_RDP_CRS_Matrix object keep track of the number
of \e serial floating point operations performed using the specified object as the \e this argument
to the function.  The Flops() function returns this number as a double precision number.  Using this 
information, in conjunction with the Petra_Time class, one can get accurate parallel performance
numbers.  The ResetFlops() function resets the floating point counter.

\warning A Petra_Map is required for the Petra_RDP_CRS_Matrix constructor.

*/    

#include "Petra_Petra.h" 
#include "Petra_Time.h" 
#include "Petra_Flops.h"
#include "Petra_BLAS.h"
#include "Petra_Map.h"
#include "Petra_Import.h"
#include "Petra_Export.h"
#include "Petra_CRS_Graph.h"
#include "Petra_RDP_Vector.h"
#include "Petra_RDP_MultiVector.h"
#include "Petra_RDP_RowMatrix.h"


class Petra_RDP_CRS_Matrix: public Petra_Flops, public Petra_BLAS, public virtual Petra_RDP_RowMatrix  {
      
  // Give ostream << function some access to private and protected data/functions.

  friend ostream& operator << (ostream& os, const Petra_RDP_CRS_Matrix& A);

 public:
  //! Petra_RDP_CRS_Matrix constuctor with variable number of indices per row.
  /*! Creates a Petra_RDP_CRS_Matrix object and allocates storage.  
    
    \param In
           CV - A Petra_DataAccess enumerated type set to Copy or View.
    \param In 
           RowMap - A Petra_Map.
    \param In
           NumEntriesPerRow - An integer array of length NumRows
	   such that NumEntriesPerRow[i] indicates the (approximate) number of entries in the ith row.
  */
  Petra_RDP_CRS_Matrix(Petra_DataAccess CV, const Petra_Map& RowMap, int *NumEntriesPerRow);
  
  //! Petra_RDP_CRS_Matrix constuctor with fixed number of indices per row.
  /*! Creates a Petra_RDP_CRS_Matrix object and allocates storage.  
    
    \param In
           CV - A Petra_DataAccess enumerated type set to Copy or View.
    \param In 
           RowMap - A Petra_Map.
    \param In
           NumEntriesPerRow - An integer that indicates the (approximate) number of entries in the each row.
	   Note that it is possible to use 0 for this value and let fill occur during the insertion phase.
	   
  */
  Petra_RDP_CRS_Matrix(Petra_DataAccess CV, const Petra_Map& RowMap, int NumEntriesPerRow);

  //! Petra_RDP_CRS_Matrix constuctor with variable number of indices per row.
  /*! Creates a Petra_RDP_CRS_Matrix object and allocates storage.  
    
    \param In
           CV - A Petra_DataAccess enumerated type set to Copy or View.
    \param In 
           RowMap - A Petra_Map.
    \param In 
           ColMap - A Petra_Map.
    \param In
           NumEntriesPerRow - An integer array of length NumRows
	   such that NumEntriesPerRow[i] indicates the (approximate) number of entries in the ith row.
  */
  Petra_RDP_CRS_Matrix(Petra_DataAccess CV, const Petra_Map& RowMap, const Petra_Map& ColMap, int *NumEntriesPerRow);
  
  //! Petra_RDP_CRS_Matrix constuctor with fixed number of indices per row.
  /*! Creates a Petra_RDP_CRS_Matrix object and allocates storage.  
    
    \param In
           CV - A Petra_DataAccess enumerated type set to Copy or View.
    \param In 
           RowMap - A Petra_Map.
    \param In 
           ColMap - A Petra_Map.
    \param In
           NumEntriesPerRow - An integer that indicates the (approximate) number of entries in the each row.
	   Note that it is possible to use 0 for this value and let fill occur during the insertion phase.
	   
  */
  Petra_RDP_CRS_Matrix(Petra_DataAccess CV, const Petra_Map& RowMap, const Petra_Map& ColMap, int NumEntriesPerRow);

  //! Construct a matrix using an existing Petra_CRS_Graph object.
  /*! Allows the nonzero structure from another matrix, or a structure that was
      constructed independently, to be used for this matrix.
    \param In
           CV - A Petra_DataAccess enumerated type set to Copy or View.
    \param In
           Graph - A Petra_CRS_Graph object, extracted from another Petra matrix object or constructed directly from
	   using the Petra_CRS_Graph constructors.
  */

  Petra_RDP_CRS_Matrix(Petra_DataAccess CV, const Petra_CRS_Graph & Graph);

  //! Copy constructor.
  Petra_RDP_CRS_Matrix(const Petra_RDP_CRS_Matrix & Matrix);

  //! Petra_RDP_CRS_Matrix Destructor
  virtual ~Petra_RDP_CRS_Matrix();

  //! Initialize all values in the graph of the matrix with constant value.
  /*!
    \param In
           Scalar - Value to use.

    \return Integer error code, set to 0 if successful.
  */
    int PutScalar(double Scalar);

  //! Insert a list of elements in a given global row of the matrix.
  /*!
    \param In
           GlobalRow - Row number (in global coordinates) to put elements.
    \param In
           NumEntries - Number of entries.
    \param In
           Values - Values to enter.
    \param In
           Indices - Global column indices corresponding to values.

    \return Integer error code, set to 0 if successful.
  */
    int InsertGlobalValues(int GlobalRow, int NumEntries, double * Values, int *Indices);

  //! Replace current values with this list of entries for a given global row of the matrix.
  /*!
    \param In
           GlobalRow - Row number (in global coordinates) to put elements.
    \param In
           NumEntries - Number of entries.
    \param In
           Values - Values to enter.
    \param In
           Indices - Global column indices corresponding to values.

    \return Integer error code, set to 0 if successful.
  */
    int ReplaceGlobalValues(int GlobalRow, int NumEntries, double * Values, int *Indices);

  //! Add this list of entries to existing values for a given global row of the matrix.
  /*!
    \param In
           GlobalRow - Row number (in global coordinates) to put elements.
    \param In
           NumEntries - Number of entries.
    \param In
           Values - Values to enter.
    \param In
           Indices - Global column indices corresponding to values.

    \return Integer error code, set to 0 if successful.
  */
    int SumIntoGlobalValues(int GlobalRow, int NumEntries, double * Values, int *Indices);

  //! Insert a list of elements in a given local row of the matrix.
  /*!
    \param In
           MyRow - Row number (in local coordinates) to put elements.
    \param In
           NumEntries - Number of entries.
    \param In
           Values - Values to enter.
    \param In
           Indices - Local column indices corresponding to values.

    \return Integer error code, set to 0 if successful.
  */
    int InsertMyValues(int MyRow, int NumEntries, double * Values, int *Indices);

  //! Replace current values with this list of entries for a given local row of the matrix.
  /*!
    \param In
           MyRow - Row number (in local coordinates) to put elements.
    \param In
           NumEntries - Number of entries.
    \param In
           Values - Values to enter.
    \param In
           Indices - Local column indices corresponding to values.

    \return Integer error code, set to 0 if successful.
  */
    int ReplaceMyValues(int MyRow, int NumEntries, double * Values, int *Indices);

  //! Add this list of entries to existing values for a given local row of the matrix.
  /*!
    \param In
           MyRow - Row number (in local coordinates) to put elements.
    \param In
           NumEntries - Number of entries.
    \param In
           Values - Values to enter.
    \param In
           Indices - Local column indices corresponding to values.

    \return Integer error code, set to 0 if successful.
  */
    int SumIntoMyValues(int MyRow, int NumEntries, double * Values, int *Indices);

    //! Signal that data entry is complete.  Perform transformations to local index space.
    /* This version of TransformToLocal assumes that the domain and range distributions are
       identical to the matrix row distributions.
    */
    int TransformToLocal();

    //! Signal that data entry is complete.  Perform transformations to local index space.
    /* This version of TransformToLocal requires the explicit specification of the domain
       and range distribution maps.  These maps are used for importing and exporting vector
       and multi-vector elements that are needed for distributed matrix computations.  For
       example, to compute y = Ax in parallel, we would specify the DomainMap as the distribution
       of the vector x and the RangeMap as the distribution of the vector y.
    \param In
           DomainMap - Map that describes the distribution of vector and multi-vectors in the
	               matrix domain.
    \param In
           RangeMap - Map that describes the distribution of vector and multi-vectors in the
	               matrix range.
    */
    int TransformToLocal(Petra_BlockMap *DomainMap, Petra_BlockMap *RangeMap);

    //! If FillComplete() has been called, this query returns true, otherwise it returns false.
    bool Filled() const {return(Graph_->Filled());};

    // Matrix data extraction routines

    //! Returns a copy of the specified global row in user-provided arrays.
    /*! 
    \param In
           GlobalRow - Global row to extract.
    \param In
	   Length - Length of Values and Indices.
    \param Out
	   NumEntries - Number of nonzero entries extracted.
    \param Out
	   Values - Extracted values for this row.
    \param Out
	   Indices - Extracted global column indices for the corresponding values.
	  
    \return Integer error code, set to 0 if successful.
  */
    int ExtractGlobalRowCopy(int GlobalRow, int Length, int & NumEntries, double *Values, int * Indices) const;

    //! Returns a copy of the specified local row in user-provided arrays.
    /*! 
    \param In
           MyRow - Local row to extract.
    \param In
	   Length - Length of Values and Indices.
    \param Out
	   NumEntries - Number of nonzero entries extracted.
    \param Out
	   Values - Extracted values for this row.
    \param Out
	   Indices - Extracted global column indices for the corresponding values.
	  
    \return Integer error code, set to 0 if successful.
  */
    int ExtractMyRowCopy(int MyRow, int Length, int & NumEntries, double *Values, int * Indices) const;

    //! Returns a copy of the specified global row values in user-provided array.
    /*! 
    \param In
           GlobalRow - Global row to extract.
    \param In
	   Length - Length of Values and Indices.
    \param Out
	   NumEntries - Number of nonzero entries extracted.
    \param Out
	   Values - Extracted values for this row.
	  
    \return Integer error code, set to 0 if successful.
  */
    int ExtractGlobalRowCopy(int GlobalRow, int Length, int & NumEntries, double *Values) const;

    //! Returns a copy of the specified local row values in user-provided array.
    /*! 
    \param In
           MyRow - Local row to extract.
    \param In
	   Length - Length of Values and Indices.
    \param Out
	   NumEntries - Number of nonzero entries extracted.
    \param Out
	   Values - Extracted values for this row.
	  
    \return Integer error code, set to 0 if successful.
  */
    int ExtractMyRowCopy(int MyRow, int Length, int & NumEntries, double *Values) const;

    //! Returns a copy of the main diagonal in a user-provided vector.
    /*! 
    \param Out
	   Diagonal - Extracted main diagonal.

    \return Integer error code, set to 0 if successful.
  */
    int ExtractDiagonalCopy(Petra_RDP_Vector & Diagonal) const;

    //! Returns a view of the specified global row values via pointers to internal data.
    /*! 
    \param In
           GlobalRow - Global row to view.
    \param Out
	   NumEntries - Number of nonzero entries extracted.
    \param Out
	   Values - Extracted values for this row.
    \param Out
	   Indices - Extracted global column indices for the corresponding values.
	  
    \return Integer error code, set to 0 if successful. Returns -1 of row not on this processor.  
            Returns -2 if matrix is not in global form (if FillComplete() has already been called).
  */
    int ExtractGlobalRowView(int GlobalRow, int& NumEntries, double *&Values, int *& Indices) const;

    //! Returns a view of the specified local row values via pointers to internal data.
    /*! 
    \param In
           MyRow - Local row to view.
    \param Out
	   NumEntries - Number of nonzero entries extracted.
    \param Out
	   Values - Extracted values for this row.
    \param Out
	   Indices - Extracted local column indices for the corresponding values.
	  
    \return Integer error code, set to 0 if successful. Returns -1 of row not on this processor.  
            Returns -2 if matrix is not in local form (if FillComplete() has \e not been called).
  */
    int ExtractMyRowView(int MyRow, int& NumEntries, double *&Values, int *& Indices) const;
    
    //! Returns a view of the specified global row values via pointers to internal data.
    /*! 
    \param In
           GlobalRow - Global row to extract.
    \param Out
	   NumEntries - Number of nonzero entries extracted.
    \param Out
	   Values - Extracted values for this row.
	  
    \return Integer error code, set to 0 if successful.
  */
    int ExtractGlobalRowView(int GlobalRow, int& NumEntries, double *&Values) const;

    //! Returns a view of the specified local row values via pointers to internal data.
    /*! 
    \param In
           MyRow - Local row to extract.
    \param Out
	   NumEntries - Number of nonzero entries extracted.
    \param Out
	   Values - Extracted values for this row.
	  
    \return Integer error code, set to 0 if successful.
  */
    int ExtractMyRowView(int MyRow, int& NumEntries, double *&Values) const;

    // Mathematical functions.


    //! Returns the result of a Petra_RDP_CRS_Matrix multiplied by a Petra_RDP_Vector x in y.
    /*! 
    \param In
	   TransA -If true, multiply by the transpose of matrix, otherwise just use matrix.
    \param In
	   x -A Petra_RDP_Vector to multiply by.
    \param Out
	   y -A Petra_RDP_Vector containing result.

    \return Integer error code, set to 0 if successful.
  */
    int Multiply(bool TransA, const Petra_RDP_Vector& x, Petra_RDP_Vector& y) const;

    //! Returns the result of a Petra_RDP_CRS_Matrix multiplied by a Petra_RDP_MultiVector X in Y.
    /*! 
    \param In
	   TransA -If true, multiply by the transpose of matrix, otherwise just use matrix.
    \param In
	   X - A Petra_RDP_MultiVector of dimension NumVectors to multiply with matrix.
    \param Out
	   Y -A Petra_RDP_MultiVector of dimension NumVectorscontaining result.

    \return Integer error code, set to 0 if successful.
  */
    int Multiply(bool TransA, const Petra_RDP_MultiVector& X, Petra_RDP_MultiVector& Y) const;

    //! Returns the result of a solve using the Petra_RDP_CRS_Matrix on a Petra_RDP_Vector x in y.
    /*! 
    \param In
	   Upper -If true, solve Ux = y, otherwise solve Lx = y.
    \param In
	   Trans -If true, solve transpose problem.
    \param In
	   UnitDiagonal -If true, assume diagonal is unit (whether it's stored or not).
    \param In
	   x -A Petra_RDP_Vector to solve for.
    \param Out
	   y -A Petra_RDP_Vector containing result.

    \return Integer error code, set to 0 if successful.
  */
    int Solve(bool Upper, bool Trans, bool UnitDiagonal, const Petra_RDP_Vector& x, Petra_RDP_Vector& y) const;

    //! Returns the result of a Petra_RDP_CRS_Matrix multiplied by a Petra_RDP_MultiVector X in Y.
    /*! 
    \param In
	   Upper -If true, solve Ux = y, otherwise solve Lx = y.
    \param In
	   Trans -If true, solve transpose problem.
    \param In
	   UnitDiagonal -If true, assume diagonal is unit (whether it's stored or not).
    \param In
	   X - A Petra_RDP_MultiVector of dimension NumVectors to solve for.
    \param Out
	   Y -A Petra_RDP_MultiVector of dimension NumVectors containing result.

    \return Integer error code, set to 0 if successful.
  */
    int Solve(bool Upper, bool Trans, bool UnitDiagonal, const Petra_RDP_MultiVector& X, Petra_RDP_MultiVector& Y) const;

    //! Computes the sum of absolute values of the rows of the Petra_RDP_CRS_Matrix, results returned in x.
    /*! The vector x will return such that x[i] will contain the inverse of sum of the absolute values of the 
        \e this matrix will be scaled such that A(i,j) = x(i)*A(i,j) where i denotes the global row number of A
        and j denotes the global column number of A.  Using the resulting vector from this function as input to LeftScale()
	will make the infinity norm of the resulting matrix exactly 1.
    \param Out
	   x -A Petra_RDP_Vector containing the row sums of the \e this matrix. 
	   \warning It is assumed that the distribution of x is the same as the rows of \e this.

    \return Integer error code, set to 0 if successful.
  */
    int InvRowSums(Petra_RDP_Vector& x) const;

    //! Scales the Petra_RDP_CRS_Matrix on the left with a Petra_RDP_Vector x.
    /*! The \e this matrix will be scaled such that A(i,j) = x(i)*A(i,j) where i denotes the row number of A
        and j denotes the column number of A.
    \param In
	   x -A Petra_RDP_Vector to solve for.

    \return Integer error code, set to 0 if successful.
  */
    int LeftScale(const Petra_RDP_Vector& x);

    //! Computes the sum of absolute values of the columns of the Petra_RDP_CRS_Matrix, results returned in x.
    /*! The vector x will return such that x[j] will contain the inverse of sum of the absolute values of the 
        \e this matrix will be sca such that A(i,j) = x(j)*A(i,j) where i denotes the global row number of A
        and j denotes the global column number of A.  Using the resulting vector from this function as input to 
	RighttScale() will make the one norm of the resulting matrix exactly 1.
    \param Out
	   x -A Petra_RDP_Vector containing the column sums of the \e this matrix. 
	   \warning It is assumed that the distribution of x is the same as the rows of \e this.

    \return Integer error code, set to 0 if successful.
  */
    int InvColSums(Petra_RDP_Vector& x) const;

    //! Scales the Petra_RDP_CRS_Matrix on the right with a Petra_RDP_Vector x.
    /*! The \e this matrix will be scaled such that A(i,j) = x(j)*A(i,j) where i denotes the global row number of A
        and j denotes the global column number of A.
    \param In
	   x -The Petra_RDP_Vector used for scaling \e this.

    \return Integer error code, set to 0 if successful.
  */
    int RightScale(const Petra_RDP_Vector& x);

    // Atribute access functions

    //! Returns the infinity norm of the global matrix.
    /* Returns the quantity \f$ \| A \|_\infty\f$ such that
       \f[\| A \|_\infty = \max_{1\lei\lem} \sum_{j=1}^n |a_{ij}| \f].
    */ 
    double NormInf() const;

    //! Returns the one norm of the global matrix.
    /* Returns the quantity \f$ \| A \|_1\f$ such that
       \f[\| A \|_1= \max_{1\lej\len} \sum_{i=1}^m |a_{ij}| \f].
    */ 
    double NormOne() const;

    //! Returns the number of nonzero entries in the global matrix.
    int NumGlobalNonzeros() const {return(Graph_->NumGlobalNonzeros());};

    //! Returns the number of global matrix rows.
    int NumGlobalRows() const {return(Graph_->NumGlobalRows_);};

    //! Returns the number of global matrix columns.
    int NumGlobalCols() const {return(Graph_->NumGlobalCols_);};

    //! Returns the number of global nonzero diagonal entries.
    int NumGlobalDiagonals() const {return(Graph_->NumGlobalDiagonals());};
    
    //! Returns the number of nonzero entries in the calling processor's portion of the matrix.
    int NumMyNonzeros() const {return(Graph_->NumMyNonzeros());};

    //! Returns the number of matrix rows owned by the calling processor.
    int NumMyRows() const {return(Graph_->NumMyRows_);};

    //! Returns the number of matrix columns owned by the calling processor.
    int NumMyCols() const {return(Graph_->NumMyCols_);};

    //! Returns the number of local nonzero diagonal entries.
    int NumMyDiagonals() const {return(Graph_->NumMyDiagonals());};

    //! Returns the current number of nonzero entries in specified global row on this processor.
    int NumGlobalEntries(int Row) const {return(Graph_->NumGlobalIndices(Row));};

    //! Returns the allocated number of nonzero entries in specified global row on this processor.
    int NumAllocatedGlobalEntries(int Row) const{return(Graph_->NumAllocatedGlobalIndices(Row));};

    //! Returns the maximu number of nonzero entries across all rows on this processor.
    int MaxNumEntries() const {return(Graph_->MaxNumIndices());};

    //! Returns the maximu number of nonzero entries across all rows on this processor.
    int GlobalMaxNumEntries() const {return(Graph_->GlobalMaxNumIndices());};

    //! Returns the current number of nonzero entries in specified local row on this processor.
    int NumMyEntries(int Row) const {return(Graph_->NumMyIndices(Row));};

    //! Returns the allocated number of nonzero entries in specified local row on this processor.
    int NumAllocatedMyEntries(int Row) const {return(Graph_->NumAllocatedMyIndices(Row));};

    //! Returns the index base for row and column indices for this graph.
    int IndexBase() const {return(Graph_->IndexBase_);};
    
    //! Sort column entries, row-by-row, in ascending order.
    int SortEntries();

    //! If SortEntries() has been called, this query returns true, otherwise it returns false.
    bool Sorted() const {return(Graph_->Sorted());};

    //! Add entries that have the same column index. Remove redundant entries from list.
    int MergeRedundantEntries();

    //! If MergeRedundantEntries() has been called, this query returns true, otherwise it returns false.
    bool NoRedundancies() const {return(Graph_->NoRedundancies());};

    //! Eliminates memory that is used for construction.  Make consecutive row index sections contiguous.
    int OptimizeStorage();

    //! If OptimizeStorage() has been called, this query returns true, otherwise it returns false.
    bool StorageOptimized() const {return(Graph_->StorageOptimized());};

    //! If matrix indices has not been transformed to local, this query returns true, otherwise it returns false.
    bool IndicesAreGlobal() const {return(Graph_->IndicesAreGlobal());};

    //! If matrix indices has been transformed to local, this query returns true, otherwise it returns false.
    bool IndicesAreLocal() const {return(Graph_->IndicesAreLocal());};

    //! If matrix indices are packed into single array (done in OptimizeStorage()) return true, otherwise false.
    bool IndicesAreContiguous() const {return(Graph_->IndicesAreContiguous());};

    //! If matrix is lower triangular, this query returns true, otherwise it returns false.
    bool LowerTriangular() const {return(Graph_->LowerTriangular());};

    //! If matrix is upper triangular, this query returns true, otherwise it returns false.
    bool UpperTriangular() const {return(Graph_->UpperTriangular());};

    //! If matrix is lower triangular, this query returns true, otherwise it returns false.
    bool NoDiagonal() const {return(Graph_->NoDiagonal());};

    //! Returns the local row index for given global row index, returns -1 if no local row for this global row.
    int LRID( int GRID) const {return(Graph_->LRID(GRID));};

    //! Returns the global row index for give local row index, returns IndexBase-1 if we don't have this local row.
    int GRID( int LRID) const {return(Graph_->GRID(LRID));};

    //! Returns the local column index for given global column index, returns -1 if no local column for this global column.
    int LCID( int GCID) const {return(Graph_->LCID(GCID));};

    //! Returns the global column index for give local column index, returns IndexBase-1 if we don't have this local column.
    int GCID( int LCID) const {return(Graph_->GCID(LCID));};
 
    //! Returns true if the GRID passed in belongs to the calling processor in this map, otherwise returns false.
    bool  MyGRID(int GRID) const {return(Graph_->MyGRID(GRID));};
   
    //! Returns true if the LRID passed in belongs to the calling processor in this map, otherwise returns false.
    bool  MyLRID(int LRID) const {return(Graph_->MyLRID(LRID));};

    //! Returns true if the GCID passed in belongs to the calling processor in this map, otherwise returns false.
    bool  MyGCID(int GCID) const {return(Graph_->MyGCID(GCID));};
   
    //! Returns true if the LRID passed in belongs to the calling processor in this map, otherwise returns false.
    bool  MyLCID(int LCID) const {return(Graph_->MyLCID(LCID));};

    //! Returns true of GID is owned by the calling processor, otherwise it returns false.
    bool MyGlobalRow(int GID) const {return(Graph_->MyGlobalRow(GID));};

    //! Returns a pointer to the Petra_CRS_Graph object associated with this matrix.
    const Petra_CRS_Graph & Graph() const {return(*Graph_);};

    //! Returns the Petra_Map object associated with the rows of this matrix.
    const Petra_Map & RowMap() const {return((Petra_Map &)Graph_->RowMap());};

    //! Returns the Petra_Map object associated with columns of this matrix.
    const Petra_Map & ColMap() const {return((Petra_Map &)Graph_->RowMap());};

    //! Returns the Petra_Map object that describes the import vector for distributed operations.
    const Petra_Map & ImportMap() const {return((Petra_Map &) Graph_->ImportMap());};

    //! Returns the Petra_Import object that contains the import operations for distributed operations.
    const Petra_Import * Importer() const {return(Graph_->Importer());};

    //! Returns the Petra_Map object that describes the export vector for distributed operations.
    const Petra_Map & ExportMap() const {return((Petra_Map &) Graph_->ExportMap());};

    //! Returns the Petra_Export object that contains the export operations for distributed operations.
    const Petra_Export * Exporter() const {return(Graph_->Exporter());};

    //! Fills a matrix with rows from a source matrix based on the specified importer.
  /*!
    \param In
           SourceMatrix - Matrix from which values are imported into the "\e this" matrix.
    \param In
           Importer - A Petra_Import object specifying the communication required.

    \param In
           CombineMode - A Petra_CombineMode enumerated type specifying how results should be combined on the 
	   receiving processor.

    \return Integer error code, set to 0 if successful.
  */
    int Import(const Petra_RDP_CRS_Matrix& SourceMatrix, const Petra_Import & Importer, 
	       Petra_CombineMode CombineMode);

    //! Fills a matrix with rows from a source matrix based on the specified exporter.
  /*!
    \param In
           SourceMatrix - Matrix from which values are imported into the "\e this" matrix.
    \param In
           Exporter - A Petra_Export object specifying the communication required.

    \param In
           CombineMode - A Petra_CombineMode enumerated type specifying how results should be combined on the 
	   receiving processor.

    \return Integer error code, set to 0 if successful.
  */
    int Export(const Petra_RDP_CRS_Matrix& SourceMatrix, 
	       const Petra_Export & Exporter, Petra_CombineMode CombineMode);

    //! Fills a matrix with rows from a source matrix based on the specified exporter.
  /*!
    \param In
           SourceMatrix - Matrix from which values are imported into the "\e this" matrix.
    \param In
           Exporter - A Petra_Export object specifying the communication required.  Communication is done
	   in reverse of an export.

    \param In
           CombineMode - A Petra_CombineMode enumerated type specifying how results should be combined on the 
	   receiving processor.

    \return Integer error code, set to 0 if successful.
  */
    int Import(const Petra_RDP_CRS_Matrix& SourceMatrix, 
	       const Petra_Export & Exporter, Petra_CombineMode CombineMode);

    //! Fills a matrix with rows from a source matrix based on the specified importer.
  /*!
    \param In
           SourceMatrix - Matrix from which values are imported into the "\e this" matrix.
    \param In
           Importer - A Petra_Import object specifying the communication required.Communication is done
	   in reverse of an import.

    \param In
           CombineMode - A Petra_CombineMode enumerated type specifying how results should be combined on the 
	   receiving processor.

    \return Integer error code, set to 0 if successful.
  */
    int Export(const Petra_RDP_CRS_Matrix& SourceMatrix, 
	       const Petra_Import & Importer, Petra_CombineMode CombineMode);

    //! Returns a pointer to the Petra_Comm communicator associated with this matrix.
    const Petra_Comm & Comm() const {return(Graph_->Comm());};

 protected:
    bool Allocated() const {return(Allocated_);};
    int SetAllocated(bool Flag) {Allocated_ = Flag; return(0);};
    double ** Values() const {return(Values_);};

  void InitializeDefaults();
  int Allocate();

  int InsertValues(int LocalRow, int NumEntries, double * Values, int *Indices);
  void SetStaticGraph(bool Flag) {StaticGraph_ = Flag;};
  int DoTransfer(const Petra_RDP_CRS_Matrix& SourceMatrix, 
		 Petra_CombineMode CombineMode,
		 int NumSameIDs, int NumPermuteIDs, int NumRemoteIDs, 
		 int NumExportIDs, 
		 int *PermuteToLIDs, int *PermuteFromLIDs, int *RemoteLIDs, 
		 int * ExportLIDs,
		 int Nsend, int Nrecv, int SizeOfPacket,
		 int & LenExports, double * & Exports, int * & IntExports,
		 int & LenImports, double * & Imports, int * & IntImports,
#ifdef PETRA_MPI
		 GSComm_Plan & Plan, 
#endif
		 bool DoReverse);

  int CopyAndPermute(Petra_RDP_CRS_Matrix & Target, 
		     const Petra_RDP_CRS_Matrix & Source,
		     int NumSameIDs, 
		     int NumPermuteIDs, int * PermuteToLIDs,
		     int *PermuteFromLIDs);
  
  int Pack(const Petra_RDP_CRS_Matrix & Source,
	   int NumSendIDs, int * SendLIDs, double * Sends, 
	   int * IntSends);
  
  int UnpackAndCombine(Petra_RDP_CRS_Matrix & Target, int SizeOfPacket,
		       int NumRecvIDs, int * RecvLIDs, 
		       double * Recvs, int * IntRecvs, Petra_CombineMode CombineMode);
  
  bool StaticGraph() {return(StaticGraph_);};
  Petra_CRS_Graph * Graph_;
  bool Allocated_;
  bool StaticGraph_;
  
  double **Values_;
  double *All_Values_;
  mutable double NormInf_;
  mutable double NormOne_;

  int NumMyRows_;
  int * NumEntriesPerRow_;
  int * NumAllocatedEntriesPerRow_;
  int ** Indices_;
  mutable Petra_RDP_MultiVector * ImportVector_;
  mutable Petra_RDP_MultiVector * ExportVector_;
  mutable int LenImports_;
  mutable int LenExports_;
  mutable double * Imports_;
  mutable double * Exports_;
  mutable int * IntImports_;
  mutable int * IntExports_;

  Petra_DataAccess CV_;

#ifdef PETRA_LEVELSCHEDULING

 public:
  //! Build level scheduling information for triangular matrix, upper or lower.
  /*! Computes level scheduling information for the current triangular graph.  
    
    \param In
           NumThreads - The number of threads intended for parallel execution.  Each level will be
	                partitioned so that each thread will get roughly the same number of nonzero
			terms at each level and thus perform approximately the same amount of work.
  */
      int ComputeLevels(int NumThreads) {return(Graph_->ComputeLevels(NumThreads));};
    //! Returns the result of a solve using the Petra_RDP_CRS_Matrix on a Petra_RDP_Vector x in y.
    /*! 
    \param In
	   Upper -If true, solve Ux = y, otherwise solve Lx = y.
    \param In
	   Trans -If true, solve transpose problem.
    \param In
	   UnitDiagonal -If true, assume diagonal is unit (whether it's stored or not).
    \param In
	   x -A Petra_RDP_Vector to solve for.
    \param Out
	   y -A Petra_RDP_Vector containing result.

    \return Integer error code, set to 0 if successful.
  */
    int LevelSolve(bool Upper, bool Trans, bool UnitDiagonal, const Petra_RDP_Vector& x, Petra_RDP_Vector& y);

    //! Returns the result of a Petra_RDP_CRS_Matrix multiplied by a Petra_RDP_MultiVector X in Y.
    /*! 
    \param In
	   Upper -If true, solve Ux = y, otherwise solve Lx = y.
    \param In
	   Trans -If true, solve transpose problem.
    \param In
	   UnitDiagonal -If true, assume diagonal is unit (whether it's stored or not).
    \param In
	   X - A Petra_RDP_MultiVector of dimension NumVectors to solve for.
    \param Out
	   Y -A Petra_RDP_MultiVector of dimension NumVectors containing result.

    \return Integer error code, set to 0 if successful.
  */
    int LevelSolve(bool Upper, bool Trans, bool UnitDiagonal, const Petra_RDP_MultiVector& X, Petra_RDP_MultiVector& Y);

#endif
};

//! << operator will work for Petra_RDP_CRS_Matrix.
ostream& operator << (ostream& os, const Petra_RDP_CRS_Matrix& A);

#endif /* _PETRA_RDP_CRS_MATRIX_H_ */
