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

ODf::Table::Table(std::string value, size_t num_rows, size_t num_cols)
{
    VecString data(num_rows * num_cols, value);
    data.insert(data.begin(), "F");
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

            auto check_characters = [&substring]()
            {
                for (char c : substring)
                {
                    if (std::isprint(c)) // if the character is printable
                    {
                        break;
                    }
                    // at this point characters in substring are invisible or whitespace
                    substring.clear(); // we clear the string of all these characters
                }
            };

            check_characters();

            start_loc = end_loc + 1; // update startong_location

            if (substring.empty())
            {
                substring = "NAN";
            }

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

ODf::Vec_UInt ODf::Table::MapFeatureNameToIndex(ODf::VecString feature_names)
{
    Vec_UInt feature_index_vec;
    for (auto f : feature_names)
    {
        auto it = std::find(list_of_features.begin(), list_of_features.end(), f);
        assert(it != list_of_features.end() && "Feature name does not exist");
        feature_index_vec.push_back(std::distance(list_of_features.begin(), it));
    }

    return feature_index_vec;
}

ODf::DType ODf::Table::GetType(std::string feature_name)
{
    size_t feature_index = MapFeatureNameToIndex({feature_name})[0];
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

std::ostream &operator<<(std::ostream &os, const ODf::VecDouble &vec_double)
{
    for (auto i : vec_double)
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

ODf::Table ODf::Table::NullOrNonNull(Vec_UInt null_columns, Vec_UInt non_null_columns)
{

    size_t max_columns = null_columns.size() > non_null_columns.size()
                             ? null_columns.size()
                             : non_null_columns.size();

    Vec_UInt accepted_rows;

    for (size_t i = 0; i < num_rows; i++)
    {
        for (size_t j = 0; j < max_columns; j++)
        {
            if (j < null_columns.size())
            {
                if (GetAt(i, null_columns[j]) == "NAN")
                {
                    accepted_rows.push_back(i);
                }
            }

            if (j < non_null_columns.size())
            {
                if (GetAt(i, non_null_columns[j]) != "NAN")
                {
                    accepted_rows.push_back(i);
                }
            }
        }
    }

    return SelectRows(accepted_rows);
}

ODf::Table ODf::Table::NullOrNonNull()
{
    return NullOrNonNull(MapFeatureNameToIndex(list_of_features), {});
}

ODf::Table ODf::Table::NullOrNonNull(VecString null_columns, VecString non_null_columns)
{
    return NullOrNonNull(MapFeatureNameToIndex(null_columns),
                         MapFeatureNameToIndex(non_null_columns));
}

Eigen::MatrixXd ODf::Table::ToMatrix()
{
    for (size_t i = 0; i < num_cols; i++)
    {
        assert(GetType(i) != DType::STR && "ensure all values are converted raw numbers");
    }

    VecDouble raw_vec;
    for (size_t i = 0; i < num_rows; i++)
    {
        for (size_t j = 0; j < num_cols; j++)
        {
            try
            {
                raw_vec.push_back(std::stod(GetAt(i, j)));
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
        }
    }

    const size_t row_length = num_rows;
    const size_t col_length = num_cols;

    Eigen::MatrixXd raw_mat;
    raw_mat.resize(num_rows, num_cols);

    for (size_t i = 0; i < num_rows; i++)
    {
        for (size_t j = 0; j < num_cols; j++)
        {
            raw_mat(i, j) = raw_vec[i * num_cols + j];
        }
    }

    return raw_mat;
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
    size_t row_size = stat_types.size();

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
                break;
            }

            case 4:
            { // first quartile
                auto column_table = SelectColumns({j});
                column_table.QuickSort(0);
                auto find_first_quartile = [&]()
                {
                    double first_quartile_term = (static_cast<double>(column_table.num_rows + 1) / 4) - 1; // -1 for index

                    if (first_quartile_term == std::floor(first_quartile_term))
                    {
                        auto first_quartile = column_table.GetAt(static_cast<long>(first_quartile_term), 0);
                        stat_vec.push_back(first_quartile);
                    }
                    else
                    {
                        long below_term = static_cast<long>(first_quartile_term);
                        long above_term = static_cast<long>(first_quartile_term) + 1;

                        double difference = first_quartile_term - static_cast<long>(first_quartile_term);

                        auto first_quartile = std::stod(column_table.GetAt(below_term, 0)) +
                                              difference *
                                                  (std::stod(column_table.GetAt(above_term, 0)) -
                                                   std::stod(column_table.GetAt(below_term, 0)));

                        stat_vec.push_back(std::to_string(first_quartile));
                    }
                };
                find_first_quartile();
                break;
            }
            case 5:
            { // second quartile
                auto column_table = SelectColumns({j});
                column_table.QuickSort(0);
                auto find_second_quartile = [&]()
                {
                    double second_quartile_term = (static_cast<double>(column_table.num_rows + 1) / 2) - 1; // -1 for index

                    if (second_quartile_term == std::floor(second_quartile_term))
                    {
                        auto second_quartile = column_table.GetAt(static_cast<long>(second_quartile_term), 0);
                        stat_vec.push_back(second_quartile);
                    }
                    else
                    {
                        long below_term = static_cast<long>(second_quartile_term);
                        long above_term = static_cast<long>(second_quartile_term) + 1;

                        double difference = second_quartile_term - static_cast<long>(second_quartile_term);

                        auto second_quartile = std::stod(column_table.GetAt(below_term, 0)) +
                                               difference *
                                                   (std::stod(column_table.GetAt(above_term, 0)) -
                                                    std::stod(column_table.GetAt(below_term, 0)));

                        stat_vec.push_back(std::to_string(second_quartile));
                    }
                };
                find_second_quartile();
                break;
            }
            case 6:
            { // third quartile
                auto column_table = SelectColumns({j});
                column_table.QuickSort(0);
                auto find_third_quartile = [&]()
                {
                    double third_quartile_term = ((static_cast<double>(column_table.num_rows + 1) * 3) / 4) - 1; // -1 for index

                    if (third_quartile_term == std::floor(third_quartile_term))
                    {
                        auto third_quartile = column_table.GetAt(static_cast<long>(third_quartile_term), 0);
                        stat_vec.push_back(third_quartile);
                    }
                    else
                    {
                        long below_term = static_cast<long>(third_quartile_term);
                        long above_term = static_cast<long>(third_quartile_term) + 1;

                        double difference = third_quartile_term - static_cast<long>(third_quartile_term);

                        auto third_quartile = std::stod(column_table.GetAt(below_term, 0)) +
                                              difference *
                                                  (std::stod(column_table.GetAt(above_term, 0)) -
                                                   std::stod(column_table.GetAt(below_term, 0)));

                        stat_vec.push_back(std::to_string(third_quartile));
                    }
                };
                find_third_quartile();
                break;
            }
            case 7:
            { // max
                stat_vec.push_back(std::to_string(SelectColumns({j}).Max()));
                break;
            }
            }
        }
    }

    auto statistics_table = Table(stat_vec, row_size, col_size);
    if (show_result == true)
    {
        std::cout << statistics_table;
    }

    return statistics_table;
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

void ODf::Table::QuickSort(size_t column_index, size_t start_index, size_t end_index,
                           OrderDirection direction)
{
    if (end_index == std::numeric_limits<size_t>::max())
        end_index = num_rows - 1;

    assert(column_index >= 0 && column_index < num_cols);
    assert(end_index < num_rows);
    assert(feature_type_info[column_index] !=
               DType::STR &&
           "Values cannot be non-numbers");

    if (!(start_index >= 0 && start_index < end_index)) // base case
        return;

    size_t pivot_index = end_index;

    auto partition = [&]() //
    {
        size_t i = 0;
        while (i < pivot_index)
        {
            if (direction == OrderDirection::ASC)
            {
                // std::cout << "Hi" << std::endl;
                if (stof(GetAt(i, column_index)) > stof(GetAt(pivot_index, column_index)) &&
                    i < pivot_index)
                {
                    auto val = GetAt(i, column_index);

                    for (size_t a = i; a < pivot_index; a++)
                    {
                        ReplaceAt(a, column_index, GetAt(a + 1, column_index));
                    }

                    ReplaceAt(pivot_index, column_index, val);
                    pivot_index--;
                }
                else
                {
                    i++;
                }
            }

            else
            {
                if (stof(GetAt(i, column_index)) < stof(GetAt(pivot_index, column_index)) &&
                    i < pivot_index)
                {
                    auto val = GetAt(i, column_index);

                    for (size_t a = i; a < pivot_index; a++)
                    {
                        ReplaceAt(a, column_index, GetAt(a + 1, column_index));
                    }

                    ReplaceAt(pivot_index, column_index, val);
                    pivot_index--;
                }
                else
                {
                    i++;
                }
            }
        }
    };

    // std::cout << *this << std::endl;
    partition();
    QuickSort(column_index, start_index, pivot_index - 1, direction);
    QuickSort(column_index, pivot_index + 1, end_index, direction);
}
