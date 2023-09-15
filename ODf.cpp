#include "ODf.hpp"

ODf::Table::Table(std::string file_path)
{
    std::ifstream csv_file;
    csv_file.open(file_path);
    // ODf::Table;

    if (csv_file)
    {
        std::string line;
        getline(csv_file, line);
        ExtractElements(line);
        num_cols = data.size();

        size_t count = 0;

        while (getline(csv_file, line))
        {
            ExtractElements(line);
            count++;
        }
        num_rows = count; // must stay above AssignTypeInfo() function
        AssignTypeInfo();
        UpdateFeatureVector();

        csv_file.close();
    }

    else
    {
        std::cerr << "\nInvalid File Directory!!!\n"
                  << std::endl;
        assert(false);
    }
}

ODf::Table::Table(ODf::VecString data, size_t num_rows, size_t num_cols)
{
    this->data = data;
    this->num_rows = num_rows;
    this->num_cols = num_cols;

    CallAllUpdaters();
}

ODf::Vec_UInt ODf::Table::GetMaxLength() const
{
    Vec_UInt max;
    for (size_t j = 0; j < num_cols; j++)
    {
        size_t val = 1;
        for (size_t i = 0; i < num_rows; i++)
        {
            std::string temp_string = data[i * num_cols + j];
            if (val < temp_string.length())
            {
                val = temp_string.length();
            }
        }
        max.push_back(val);
    }

    return max;
}

void ODf::Table::ExtractElements(std::string line)
{
    size_t start_loc = 0;
    size_t end_loc;
    size_t size; // size of substring
    std::string substring;

    line += ","; // appended a comma to solve the issue of selecting last number in string

    for (size_t i = 0; i < line.size(); i++)
    {
        if (line[i] == ',')
        {
            end_loc = i;

            size = (end_loc - start_loc);
            substring = line.substr(start_loc, size);
            start_loc = end_loc + 1;

            data.push_back(substring);
        }
    }
}

void ODf::Table::AssignTypeInfo()
{
    feature_type_info.clear();
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<size_t> dist(0, num_rows);
    size_t max_row;

    if (num_rows < 100)
        max_row = num_rows;
    else
        max_row = 100;

    for (size_t j = 0; j < num_cols; j++)
    {
        DType dtype = DType::STR;
        size_t float_count = 0;
        size_t int_count = 0;
        size_t string_count = 0;

        for (size_t i = 1; i < max_row + 1; i++)
        {
            size_t random_row = dist(gen);
            try
            {
                std::string string_val = data[i * num_cols + j];
                int check = std::stoi(string_val);

                int_count++;
                if (string_val.find('.') != std::string::npos)
                {
                    float_count++;
                }
            }
            catch (const std::exception e)
            {
                string_count++;
                // std::cerr << e.what() << '\n';
            }
        }
        if (string_count < int_count)
        {
            dtype = (float_count > 0) ? DType::FLOAT : DType::INT;
        }

        feature_type_info.push_back(dtype);
    }
}

void ODf::Table::UpdateFeatureVector()
{
    list_of_features.clear();
    for (size_t j = 0; j < num_cols; j++)
    {
        list_of_features.push_back(data[j]);
    }
}

void ODf::Table::CallAllUpdaters()
{
    AssignTypeInfo();
    UpdateFeatureVector();
}

size_t ODf::Table::MapFeatureNameToIndex(std::string feature_name)
{
    auto it = std::find(list_of_features.begin(), list_of_features.end(), feature_name);

    assert(it != list_of_features.end() && "Feature name does not exist");

    return std::distance(list_of_features.begin(), it);
}

ODf::DType ODf::Table::GetType(std::string feature_name)
{
    // auto it = std::find(list_of_features.begin(), list_of_features.end(), feature_name);

    // assert(it != list_of_features.end() && "Feature name does not exist");

    size_t feature_index = MapFeatureNameToIndex(feature_name);

    return GetType(feature_index);
}
ODf::DType ODf::Table::GetType(size_t index_loc)
{
    assert(index_loc >= 0 && index_loc < feature_type_info.size());
    return feature_type_info[index_loc];
}

std::ostream &ODf::operator<<(std::ostream &os, const ODf::Table &Table)
{
    Vec_UInt max = Table.GetMaxLength();
    std::string row_spacing = std::string(3, ' ');
    os << "\n";
    for (size_t i = 0; i < Table.num_rows + 1; i++)
    {
        if (i > 0)
        {
            std::string index_id = "[" + std::to_string(i - 1) + "]" + std::string(5, ' ');
            std::string begin_spacing = std::string(20 - index_id.length(), ' ');
            os << begin_spacing << index_id;
        }
        else
        {
            std::string begin_spacing = std::string(20, ' ');
            os << begin_spacing;
        }
        for (size_t j = 0; j < Table.num_cols; j++)
        {
            size_t dashed_length = 0;
            std::string string_val = Table.data[i * Table.num_cols + j];
            size_t elem_length = string_val.length();
            std::string spacing = "";
            if (elem_length < max[j])
            {
                spacing = std::string(max[j] - elem_length, ' ');
            }
            os << string_val + spacing + row_spacing;
        }
        os << "\n";
    }
    os << "Sample count: " << Table.num_rows << "\n";
    os << "Feature count: " << Table.num_cols << "\n\n";
    return os;
}

std::ostream &operator<<(std::ostream &os, const ODf::Vec_UInt &vec_size_t)
{
    for (auto i : vec_size_t)
    {
        os << i << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const ODf::VecString &vec_string)
{
    for (auto i : vec_string)
    {
        os << i << "\n";
    }

    return os;
}

ODf::Table ODf::Table::Cut(size_t r1, size_t r2, size_t c1, size_t c2)
{
    assert(r2 > r1 && " r2 cannot be less than r1");
    assert(c2 > c1 && " c2 cannot be less than c1");
    assert(r2 < this->num_rows + 1 && "Row range exceeded");
    assert(c2 < this->num_cols + 1 && "Column range exceeded");
    assert(r1 != r2 && "r1 cannot be equal to r2");
    assert(c1 != c2 && "c1 cannot be equal to c2");

    size_t sliced_rows = r2 - r1;
    size_t sliced_cols = c2 - c1;

    // r1 += 1; // so as not to access feaature names
    // r2
    ODf::VecString slice;
    for (size_t j = c1; j < c2; j++)
    {
        slice.push_back(this->data[j]);
    }
    for (size_t i = r1 + 1; i < r2 + 1; i++)
    {
        for (size_t j = c1; j < c2; j++)
        {
            slice.push_back(this->data[i * this->num_cols + j]);
        }
    }

    Table new_table(slice, sliced_rows, sliced_cols);
    new_table.CallAllUpdaters();
    return new_table;
}

ODf::Table ODf::Table::RowCut(size_t r1, size_t r2)
{
    assert(r2 > r1 && " r2 cannot be less than r1");
    assert((int)r1 > -1 && r2 < this->num_rows + 1 && "Row range exceeded");
    assert(r1 != r2 && "r1 cannot be equal to r2");

    size_t sliced_rows = r2 - r1;
    size_t sliced_cols = num_cols;

    // r1 += 1; // so as not to access feaature names

    ODf::VecString slice;
    for (size_t j = 0; j < num_cols; j++)
    {
        slice.push_back(data[j]);
    }
    for (size_t i = r1 + 1; i < r2 + 1; i++)
    {
        for (size_t j = 0; j < num_cols; j++)
        {
            slice.push_back(this->data[i * num_cols + j]);
        }
    }
    Table new_table(slice, sliced_rows, sliced_cols);
    new_table.CallAllUpdaters();
    return new_table;
}

ODf::Table ODf::Table::ColumnCut(size_t c1, size_t c2)
{
    assert(c2 > c1 && " c2 cannot be less than c1");
    assert((int)c1 > -1 && c2 < this->num_cols + 1 && "Column range exceeded");
    assert(c1 != c2 && "c1 cannot be equal to c2");

    size_t sliced_rows = this->num_rows;
    size_t sliced_cols = c2 - c1;

    ODf::VecString slice;
    for (size_t j = c1; j < c2; j++)
    {
        slice.push_back(this->data[j]);
    }
    for (size_t i = 1; i < this->num_rows + 1; i++)
    {
        for (size_t j = c1; j < c2; j++)
        {
            slice.push_back(this->data[i * this->num_cols + j]);
        }
    }
    Table new_table(slice, sliced_rows, sliced_cols);
    new_table.CallAllUpdaters();
    return new_table;
}

ODf::Table ODf::Table::SelectColumns(Vec_UInt col_index_select_vec)
{
    ODf::VecString new_data;
    for (size_t i = 0; i < this->num_rows + 1; i++)
    {
        for (auto j : col_index_select_vec)
        {
            new_data.push_back(this->data[i * this->num_cols + j]);
        }
    }

    Table new_table(new_data, this->num_rows, col_index_select_vec.size());
    new_table.CallAllUpdaters();
    return new_table;
}

ODf::Table ODf::Table::SelectColumns(ODf::VecString features_to_select)
{
    Vec_UInt col_index_select_vec;
    for (auto feature : features_to_select)
    {
        auto it = find(this->list_of_features.begin(), this->list_of_features.end(), feature);

        if (it != this->list_of_features.end())
        {
            size_t index = it - this->list_of_features.begin();
            col_index_select_vec.push_back(index);
        }

        else
        {
            assert(false && "\nIndex out of range!");
        }
    }
    return this->SelectColumns(col_index_select_vec);
}

ODf::Table ODf::Table::SelectRows(Vec_UInt row_index_select_vec)
{
    ODf::VecString new_data;
    for (size_t j = 0; j < this->num_cols; j++)
    {
        new_data.push_back(this->data[j]);
    }
    for (auto i : row_index_select_vec)
    {
        for (size_t j = 0; j < this->num_cols; j++)
        {
            new_data.push_back(this->data[(i + 1) * this->num_cols + j]);
        }
    }

    Table new_table(new_data, row_index_select_vec.size(), this->num_cols);
    new_table.CallAllUpdaters();
    return new_table;
}

void ODf::Table::ToCSV(std::string file_name, std::string directory)
{
    // std::cout << "Starting..." << std::endl;
    std::ofstream file;
    std::string path = directory + "/" + file_name + ".csv";
    size_t inc = 0;
    file.open(path);
    if (file.is_open())
    {
        std::cout << "\nFile open!" << std::endl;
        std::cout << "Begining transfer....." << std::endl;
        for (size_t i = 0; i < this->num_rows + 1; i++)
        {
            for (size_t j = 0; j < this->num_cols; j++)
            {
                if (j == num_cols - 1)
                    file << data[i * num_cols + j];
                else
                    file << data[i * num_cols + j] + ",";
            }
            file << "\n";
        }

        std::cout << "File completed!" << std::endl;
        file.close();
    }
    else
    {
        assert(false && "Invalid file path");
    }
}

void ODf::Table::RenameColumn(size_t col_index, std::string name)
{
    assert(col_index < this->num_cols && " col_index provided exceeds range");
    this->data[col_index] = name;
    UpdateFeatureVector();
}

void ODf::Table::UniqueCounts(size_t col_index)
{
    assert(col_index < this->num_cols && " column index provided exceeds range");
    std::unordered_map<std::string, size_t> occurrenceMap;
    for (size_t i = 1; i < num_rows + 1; i++)
    {
        // If the string is already in the map, increment its occurrence count
        size_t loc = i * num_cols + col_index;
        if (occurrenceMap.find(this->data[loc]) != occurrenceMap.end())
        {
            occurrenceMap[this->data[loc]]++;
        }
        else
        {
            // If the string is not in the map, add it with an occurrence count of 1
            occurrenceMap[this->data[loc]] = 1;
        }
    }
    std::cout << std::string(9 + list_of_features[col_index].length(), '-') << std::endl;
    std::cout << "Feature: " << list_of_features[col_index] << std::endl;
    std::cout << std::string(9 + list_of_features[col_index].length(), '-') << std::endl;

    for (const auto &entry : occurrenceMap)
    {
        std::cout << entry.first << ": " << entry.second << std::endl;
    }
}

void ODf::Table::UniqueCounts(std::string feature_name)
{
    auto it = find(list_of_features.begin(), list_of_features.end(), feature_name);
    if (it != list_of_features.end())
    {
        size_t index = it - list_of_features.begin();
        UniqueCounts(index);
    }
    else
    {
        std::cerr << "Invalid feature name!" << std::endl;
    }
}

void ODf::Table::Shuffle(size_t random_state)
{ // fisher yates
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(1, num_rows);
    for (size_t i = 0; i < this->num_rows - 1; i++)
    {
        size_t random_row = dist(gen);

        for (size_t j = 0; j < this->num_cols; j++)
        {
            std::string temp_val = this->data[(i + 1) * this->num_cols + j];
            this->data[(i + 1) * this->num_cols + j] = this->data[(random_row) * this->num_cols + j];
            this->data[(random_row + 1) * this->num_cols + j] = temp_val;
        }
    }
}

void ODf::Table::ReplaceAt(size_t i, size_t j, std::string string_val)
{
    this->data[(i + 1) * this->num_cols + j] = string_val;
}

const void ODf::Table::Info()
{
    size_t byte_size = (sizeof(size_t) * this->data.size());
    std::string unit = "bytes";
    if (byte_size > 1024)
    {
        byte_size /= 1024; // Kilobyte
        unit = "kilobytes";
        if (byte_size > 1024)
        {
            byte_size /= 1024; // Megabyte
            unit = "megabytes";
            if (byte_size > 1024)
            {
                byte_size /= 1024; // Gigaabyte
                unit = "gigabytes";
            }
        }
    }

    printf("Memory: %zu %s: \n", byte_size, unit.c_str());
    printf("Range Index: %zu entries, 0 to %zu\n", num_rows, num_rows - 1);
    printf("Data Columns: %zu columns:\n", num_cols);

    for (size_t j = 0; j < num_cols; j++)
    {
        // printf("[%zu] %s: ", j, list_of_features[j].c_str());
        size_t non_null_count = 0;
        size_t null_count = 0;
        if (feature_type_info[j] == DType::INT)
        {
            for (size_t i = 1; i < num_rows + 1; i++)
            {
                try
                {
                    std::ignore = std::stoi(data[i * num_cols + j]);
                    non_null_count++;
                }
                catch (const std::exception e)
                {
                    null_count++;
                }
            }
        }

        else if (feature_type_info[j] == DType::FLOAT)
        {
            for (size_t i = 1; i < num_rows + 1; i++)
            {
                try
                {
                    std::ignore = std::stof(data[i * num_cols + j]);
                    non_null_count++;
                }
                catch (const std::exception e)
                {
                    null_count++;
                }
            }
        }

        else
        {
            for (size_t i = 1; i < num_rows + 1; i++)
            {
                non_null_count++;
            }
        }

        auto string_type = [](DType dtype) -> std::string
        {
            if (dtype == DType::FLOAT)
                return "FLOAT";
            if (dtype == DType::INT)
                return "INT";

            return "STR";
        };

        printf("\n[%zu] %s: non-null count: %zu, null count: %zu, Type: %s\n\n",
               j, list_of_features[j].c_str(), non_null_count, null_count, string_type(feature_type_info[j]).c_str());
    }
}

const double ODf::Table::Mean()
{
    assert(num_cols == 1); // ensure its 1 dimensional
    assert(feature_type_info[0] != DType::STR && "Values cannot be non-numbers");
    double sum = 0;
    for (size_t i = 1; i < num_rows + 1; i++)
    {
        sum += std::stod(data[i]);
    }
    return sum / num_rows;
}

const double ODf::Table::StandardDev()
{
    assert(num_cols == 1); // ensure its 1 dimensional
    assert(feature_type_info[0] != DType::STR && "Values cannot be non-numbers");
    double sum_sqrt_difference = 0;
    double mean = Mean();

    for (size_t n = num_cols; n < data.size(); n++)
    {
        sum_sqrt_difference += pow(std::stof(this->data[n]) - mean, 2);
    }
    return sqrt(sum_sqrt_difference / (data.size() - num_cols));
}

const double ODf::Table::Max()
{
    assert(num_cols == 1); // ensure its 1 dimensional
    assert(feature_type_info[0] != DType::STR && "Values cannot be non-numbers");
    double max = std::stod(data[num_cols]);
    for (size_t n = num_cols; n < data.size(); n++)
    {
        max = (max > std::stod(data[n])) ? max : std::stod(data[n]);
    }

    return max;
}
const double ODf::Table::Min()
{
    assert(num_cols == 1); // ensure its 1 dimensional
    assert(feature_type_info[0] != DType::STR && "Values cannot be non-numbers");
    double min = std::stod(data[num_cols]);
    for (size_t n = num_cols; n < data.size(); n++)
    {
        min = (min < std::stod(data[n])) ? min : std::stod(data[n]);
    }

    return min;
}

ODf::Table ODf::Table::Statistics(bool show_result)
{
    ODf::VecString stat_vec = {"Statistics"}; // create vector to hold statistic and adding "Statistics" header
    auto feature_list = FeatureNameVector();
    stat_vec.insert(std::end(stat_vec), std::begin(feature_list),
                    std::end(feature_list));

    ODf::VecString stat_types = {"count",
                                 "mean",
                                 "std",
                                 "min",
                                 "25%",
                                 "50%",
                                 "75%",
                                 "max"};

    size_t col_size = this->num_cols + 1;

    for (size_t i = 0; i < stat_types.size(); i++)
    {
        stat_vec.push_back(stat_types[i]);
        for (size_t j = 0; j < col_size - 1; j++)
        {
            DType type_info = SelectColumns({j}).feature_type_info[0];
            switch (i)
            {
            case 0:
            { // count
                size_t non_null_count = 0;
                if (type_info != DType::STR)
                {
                    for (size_t a = 1; a < num_rows + 1; a++)
                    {
                        try
                        {
                            std::ignore = std::stoi(data[a * num_cols]);
                            non_null_count++;
                        }
                        catch (const std::exception e)
                        { // no actions taken
                        }
                    }
                }
                else
                {
                    non_null_count = num_rows;
                }
                stat_vec.push_back(std::to_string(non_null_count));
                break;
            }
            case 1:
            { // mean
                stat_vec.push_back(std::to_string(SelectColumns({j}).Mean()));
                break;
            }
            case 2:
            { // standard deviation
                stat_vec.push_back(std::to_string(SelectColumns({j}).StandardDev()));
                break;
            }
            case 3:
            { // min
                stat_vec.push_back(std::to_string(SelectColumns({j}).Min()));
                std::cout << "Min: " << SelectColumns({j}).Min() << std::endl;
                break;
            }

            case 4:
            { // first quartile

                break;
            }
            case 5:
            { // second quartile

                break;
            }
            case 6:
            { // third quartile

                break;
            }
            case 7:
            { // max
                stat_vec.push_back(std::to_string(SelectColumns({j}).Max()));
                std::cout << "Max: " << SelectColumns({j}).Max() << std::endl;
                break;
            }
            }
        }
    }
    // return Table("jjj");
}

size_t ODf::Table::RowSize()
{
    return num_rows;
}

size_t ODf::Table::ColumnSize()
{
    return num_cols;
}

const std::string ODf::Table::GetAt(size_t i, size_t j)
{
    assert(i < num_rows && "Row index out of range");
    assert(j < num_cols && "Column index out of range");
    i++;
    return this->data[i * this->num_cols + j];
}

const ODf::VecString ODf::Table::FeatureNameVector()
{
    return this->list_of_features;
}

const ODf::VecString ODf::Table::GetVectorData()
{
    return this->data;
}

ODf::Table ODf::RowConcat(Table t1, Table t2)
{
    assert(t1.ColumnSize() == t2.ColumnSize());
    ODf::VecString data_1 = t1.GetVectorData();
    ODf::VecString data_2 = t2.GetVectorData();
    for (size_t j = 0; j < t2.ColumnSize(); j++)
    {
        data_2.erase(data_2.begin());
    }
    data_1.insert(data_1.end(), data_2.begin(), data_2.end());

    Table concatenated_table(data_1, t1.RowSize() + t2.RowSize(), t1.ColumnSize());
    concatenated_table.CallAllUpdaters();
    return concatenated_table;
}

ODf::Table ODf::ColumnConcat(Table t1, Table t2)
{
    VecString col_concat_data;
    assert(t1.RowSize() == t2.RowSize());

    for (auto t2_feature_name : t2.FeatureNameVector())
    {
        VecString t1_feature_vec = t1.FeatureNameVector();
        if (std::find(t1_feature_vec.begin(), t1_feature_vec.end(), t2_feature_name) != t1_feature_vec.end())
        {
            assert(false && "Feature name in Table t2 colliding with Table t1");
        }
    }

    for (size_t i = 0; i < t1.RowSize() + 1; i++)
    {
        VecString t1_data = t1.GetVectorData();
        VecString t2_data = t2.GetVectorData();
        size_t t1_col_size = t1.ColumnSize();
        size_t t2_col_size = t2.ColumnSize();

        for (size_t j = 0; j < t1_col_size + t2_col_size; j++)
        {
            if (j < t1_col_size)
            {
                col_concat_data.push_back(t1_data[i * t1_col_size + j]);
            }
            else
            {
                col_concat_data.push_back(t2_data[i * t2_col_size + (j - t1_col_size)]);
            }
        }
    }

    Table concat_table(col_concat_data, t1.RowSize(), t1.ColumnSize() + t2.ColumnSize());
    concat_table.CallAllUpdaters();
    return concat_table;
}

void ODf::Table::QuickSort(size_t column_index, size_t start, size_t end)
{
    if (end == std::numeric_limits<size_t>::max())
    {
        end = num_rows - 1;
    }
    // std::cout << "Column index:" << column_index << std::endl;
    // std::cout << "Num columns: " << num_cols << std::endl;

    assert(column_index >= 0 && column_index < num_cols); // ensure its 1 dimensional
    assert(end < num_rows);
    assert(start >= 0 && start < end);
    assert(feature_type_info[column_index] !=
               DType::STR &&
           "Values cannot be non-numbers");

    size_t pivot_index = end;
    bool shift_occurred = false;

    auto shift = [&](size_t greater_val_index) // lambda to shift
    {
        if (greater_val_index < pivot_index &&
            std::stof(GetAt(greater_val_index, column_index)) >=
                std::stof(GetAt(pivot_index, column_index)))
        {
            shift_occurred = true;
            auto greater_value = GetAt(greater_val_index, column_index);

            for (size_t i = greater_val_index; i < end + 1; i++)
            {
                if (i == end)
                {
                    ReplaceAt(i, column_index, greater_value);
                    continue;
                }

                ReplaceAt(i, column_index, GetAt(i + 1, column_index));
            }
            pivot_index--; // update pivot position
        }
    };

    for (size_t i = start; i < end; i++)
    {
        if (i == pivot_index)
        {
            break;
        }

        shift(i);
    }
    if (shift_occurred == false)
    {
        std::cout << "shift did not occur" << std::endl;
        return;
    }

    std::cout << *this << std::endl;
    // printf("Start: %zu, End: %zu\n", start, end);
    // printf("Pivot_index: %zu\n", pivot_index);

    // if (start == pivot_index)
    //     return;

    QuickSort(column_index, 0, pivot_index - 1);
    QuickSort(column_index, pivot_index + 1, end);
}

#include "ODf.hpp"
int main()
{
    ODf::Table *a = new ODf::Table("DataSource/tvmarketing.csv");
    auto b1 = a->Cut(5, 11, 0, 2);
    // b1.ReplaceAt(0, 0, "100");
    // b1.ReplaceAt(3, 0, "53");
    std::cout << b1;
    b1.QuickSort(0);
    std::cout << b1;

    // std::cout << b1.GetAt(1, 1) << std::endl;

    // b1.Statistics();
    // c1.Statistics();
}