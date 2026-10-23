#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>

// Лабораторная работа №1, задание 7 (высокий уровень), вариант 2.
// Работа с текстовыми файлами, содержащими матрицы размера m x n.

using Matrix = std::vector<std::vector<double>>;

struct MatrixFile {
    std::size_t rows = 0;
    std::size_t columns = 0;
    std::vector<Matrix> matrices;
};

// Формат файла:
// количество_матриц количество_строк количество_столбцов
// элементы первой матрицы по строкам
// элементы второй матрицы по строкам
// ...
MatrixFile readMatrices(const std::string& filename) {
    std::ifstream input(filename);
    if (!input.is_open()) {
        throw std::runtime_error("Cannot open input file: " + filename);
    }

    long long count = 0;
    long long rows = 0;
    long long columns = 0;
    if (!(input >> count >> rows >> columns) ||
        count < 0 || rows <= 0 || columns <= 0) {
        throw std::runtime_error("Invalid header in file: " + filename);
    }

    const auto rowCount = static_cast<std::size_t>(rows);
    const auto columnCount = static_cast<std::size_t>(columns);
    if (rowCount > (std::numeric_limits<std::size_t>::max)() / columnCount) {
        throw std::runtime_error("Matrix dimensions are too large: " + filename);
    }

    MatrixFile data;
    data.rows = rowCount;
    data.columns = columnCount;

    for (long long i = 0; i < count; ++i) {
        Matrix matrix(rowCount, std::vector<double>(columnCount));
        for (std::size_t r = 0; r < rowCount; ++r) {
            for (std::size_t c = 0; c < columnCount; ++c) {
                if (!(input >> matrix[r][c]) || !std::isfinite(matrix[r][c])) {
                    throw std::runtime_error(
                        "Invalid or missing element in matrix " +
                        std::to_string(i + 1) + " of file: " + filename);
                }
            }
        }
        data.matrices.push_back(std::move(matrix));
    }

    // Проверка, что после объявленных матриц нет лишних данных.
    input >> std::ws;
    if (input.peek() != std::char_traits<char>::eof()) {
        throw std::runtime_error("Unexpected extra data in file: " + filename);
    }
    return data;
}

void writeMatrices(const std::string& filename, const MatrixFile& data) {
    std::ofstream output(filename, std::ios::trunc);
    if (!output.is_open()) {
        throw std::runtime_error("Cannot open output file: " + filename);
    }

    output << data.matrices.size() << ' ' << data.rows << ' '
           << data.columns << '\n';
    output << std::setprecision(std::numeric_limits<double>::max_digits10);

    for (std::size_t i = 0; i < data.matrices.size(); ++i) {
        for (const auto& row : data.matrices[i]) {
            for (std::size_t j = 0; j < row.size(); ++j) {
                if (j != 0) output << ' ';
                output << row[j];
            }
            output << '\n';
        }
        if (i + 1 != data.matrices.size()) output << '\n';
    }

    output.close();
    if (!output) {
        throw std::runtime_error("Error writing file: " + filename);
    }
}

void printMatrices(const std::string& title, const MatrixFile& data) {
    std::cout << "\n=== " << title << " ===\n";
    std::cout << "Number of matrices: " << data.matrices.size()
              << ", dimensions: " << data.rows << " x " << data.columns << '\n';

    for (std::size_t i = 0; i < data.matrices.size(); ++i) {
        std::cout << "Matrix " << i + 1 << ":\n";
        for (const auto& row : data.matrices[i]) {
            for (std::size_t j = 0; j < row.size(); ++j) {
                if (j != 0) std::cout << '\t';
                std::cout << std::setprecision(10) << row[j];
            }
            std::cout << '\n';
        }
    }
    if (data.matrices.empty()) {
        std::cout << "(no matrices)\n";
    }
}

int main(int argc, char* argv[]) {
    try {
        if (argc != 1 && argc != 4) {
            std::cerr << "Usage: " << argv[0]
                      << " [first.txt second.txt third.txt]\n";
            return 1;
        }

        const std::string firstName = argc == 4 ? argv[1] : "first.txt";
        const std::string secondName = argc == 4 ? argv[2] : "second.txt";
        const std::string thirdName = argc == 4 ? argv[3] : "third.txt";

        if (firstName == secondName || firstName == thirdName ||
            secondName == thirdName) {
            throw std::runtime_error("All three file names must be different.");
        }

        // Вначале полностью считываем оба файла и проверяем их содержимое.
        MatrixFile first = readMatrices(firstName);
        MatrixFile second = readMatrices(secondName);
        if (first.rows != second.rows || first.columns != second.columns) {
            throw std::runtime_error("Matrix dimensions in the files differ.");
        }

        MatrixFile third;
        third.rows = first.rows;
        third.columns = first.columns;

        // Лишними считаем последние матрицы большего файла.
        // Порядок оставшихся и перенесённых матриц сохраняется.
        bool firstChanged = false;
        bool secondChanged = false;
        if (first.matrices.size() > second.matrices.size()) {
            const std::size_t keep = second.matrices.size();
            third.matrices.assign(first.matrices.begin() + keep,
                                  first.matrices.end());
            first.matrices.resize(keep);
            firstChanged = true;
        } else if (second.matrices.size() > first.matrices.size()) {
            const std::size_t keep = first.matrices.size();
            third.matrices.assign(second.matrices.begin() + keep,
                                  second.matrices.end());
            second.matrices.resize(keep);
            secondChanged = true;
        }

        // Сначала сохраняем лишние матрицы в третий файл,
        // только затем изменяем исходный файл с большим числом матриц.
        writeMatrices(thirdName, third);
        if (firstChanged) writeMatrices(firstName, first);
        if (secondChanged) writeMatrices(secondName, second);

        // Содержимое трёх файлов после обработки.
        printMatrices("FIRST FILE: " + firstName, first);
        printMatrices("SECOND FILE: " + secondName, second);
        printMatrices("THIRD FILE: " + thirdName, third);
        std::cout << "\nCompleted successfully.\n";
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
