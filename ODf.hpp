#pragma once
#include <vector>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <cassert>
#include <unordered_map>
#include <algorithm>
#include "eigen/Dense"

namespace ODf
{
    enum class DType
    {
        INT,
        FLOAT,
        STR
    };

    enum class OrderDirection
    {
        ASC,
        DSC
    };

    typedef std::vector<std::string> VecString;
    typedef std::vector<std::size_t> Vec_UInt;
    typedef std::vector<double> VecDouble;
    typedef std::vector<DType> Vec_DType;

    typedef struct Table
    {
    private:
        size_t num_rows;
        size_t num_cols;
        VecString data;
        Vec_DType feature_type_info;
        VecString list_of_features;

        Vec_UInt GetMaxLength() const;

        void ExtractElements(std::string line); // for reading CSVs

        // Updaters
        void AssignTypeInfo();
        void UpdateFeatureVector();

    public:
        Table(std::string file_path);
        Table(VecString data, size_t num_rows, size_t num_cols);
        Table(std::vector<VecString> data);

        Table Cut(size_t r1, size_t r2, size_t c1, size_t c2);
        Table RowCut(size_t r1, size_t r2);
        Table ColumnCut(size_t c1, size_t c2);
        Table Statistics(bool = false);
        Table SelectColumns(VecString features_to_select);
        Table NullOrNonNull(VecString null_columns, VecString non_null_columns);
        Table SelectColumns(Vec_UInt col_index_select_vec);
        Table SelectRows(Vec_UInt row_index_select_vec);
        Table NullOrNonNull(Vec_UInt null_columns, Vec_UInt non_null_columns);
        Table NullOrNonNull();

        Eigen::MatrixXd ToMatrix();

        DType GetType(std::string feature_name);
        DType GetType(size_t index_loc);

        size_t RowSize();
        size_t ColumnSize();

        Vec_UInt MapFeatureNameToIndex(VecString feature_names);
        // Table GroupBy(size_t column, std::string value);
        const double Mean();
        const double StandardDev();
        const double Max();
        const double Min();

        void CallAllUpdaters(); // calls all updaters
        void ToCSV(std::string file_name, std::string directory);
        void RenameColumn(size_t col_index, std::string name);
        void UniqueCounts(size_t col_index);
        void UniqueCounts(std::string feature_name);
        void Shuffle(size_t random_state);
        void ReplaceAt(size_t i, size_t j, std::string string_val);
        void QuickSort(size_t column_index, size_t start_index = 0,
                       size_t end_index = std::numeric_limits<size_t>::max(),
                       OrderDirection direction = OrderDirection::ASC);

        const void Info();
        const std::string GetAt(size_t i, size_t j);
        const VecString FeatureNameVector();
        const VecString GetVectorData();
        friend std::ostream &operator<<(std::ostream &os, const Table &tables);
    } Table;

    Table RowConcat(Table t1, Table t2);
    Table ColumnConcat(Table t1, Table t2);

    // Table Difference(Table t1, Table t2);
    // Table Add(Table t1, Table t2);
    // Table ElemMult(Table t1, Table t2);
    // Table ScalarMult(Table t, float val);
    // Table ScalarMult(Table t, long val);
    // Table ElemDiv(Table t1, Table t2);
    // Table ScalarDiv(Table t, float val);
}

std::ostream &operator<<(std::ostream &os, const ODf::Vec_UInt &vec_size_t);
std::ostream &operator<<(std::ostream &os, const ODf::VecString &vec_string);
std::ostream &operator<<(std::ostream &os, const ODf::VecDouble &vec_double);
// Table DropRow(Vec_UInt row_drop_vec);
// Table DropRow(std::vector<std::string> rows_to_drop);
