# ODf - OurDataframe
A dataframe library written entirely in C++, inspired by Python's pandas library.<br>
This project was built to support the MLA library enabling users to carry out:<br>
- Data cleaning and preparation
- Data tranformation(cutting either by rows or columns or both, removing and adding values)
- Reading and writing data from csv files
- Exploratory analysis(collecting info on nulls/non-nulls in dataset, summary statistics, etc)
- Arithmetic operations with data(Int or Float)
- Converting dataframes to raw formats

Functions are self-explanatory by their names. Check the our-dataframe.hpp file for functions available

## Examples
<strong>Note the name of header file has been updated from "our-dataframe.hpp" to "ODf.hpp". <strong>
 <br>
### Reading and Cutting data
![Alt text](image.png)

#### Result:
![Alt text](image-1.png)

### Select data by columns/rows
You can select columns via their column names or column index:
![Alt text](image-2.png)

#### Result:
![Alt text](image-3.png)
