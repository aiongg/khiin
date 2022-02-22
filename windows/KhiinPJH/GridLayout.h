#pragma once

#include <algorithm>
#include <numeric>
#include <vector>

#include "Geometry.h"

namespace khiin::geometry {

struct GridLayout {
    GridLayout() = default;
    GridLayout(int num_rows, int num_cols, int min_col_width = 0) : rows_(num_rows), cols_(num_cols) {
        col_widths_ = std::vector<int>(num_cols, min_col_width);
    };

    inline void EnsureColumnWidth(size_t column_index, int width) {
        assert(column_index < col_widths_.size());
        col_widths_[column_index] = std::max<int>(col_widths_[column_index], width);
    }

    inline void EnsureRowHeight(int height) {
        row_height_ = std::max<int>(row_height_, height);
    }

    inline void SetRowPadding(int row_padding) {
        row_pad_ = row_padding;
    }

    inline int rows() {
        return rows_;
    }

    inline int cols() {
        return cols_;
    }

    inline Rect GetCellRect(int row, int col) {
        assert(row <= rows_);
        assert(col <= cols_);
        auto left = std::accumulate(col_widths_.begin(), col_widths_.begin() + col, 0);
        auto top = row * (row_height_ + row_pad_);
        auto width = col_widths_[col];
        auto height = row_height_ + row_pad_;

        return Rect(Point{left, top}, width, height);
    }

    inline Size GetGridSize() {
        auto width = std::accumulate(col_widths_.begin(), col_widths_.end(), 0) + row_pad_ * 2;
        auto height = rows_ * row_height_ + (rows_ + 1) * row_pad_;
        return Size{width, height};
    }

    inline std::pair<int, int> HitTest(Point const &pt) {
        auto size = GetGridSize();
        if (0 <= pt.x && pt.x <= size.width && 0 <= pt.y && pt.y <= size.height) {
            auto row = std::div(pt.y, row_height_ + row_pad_).quot;
            auto col = 0;
            auto acc_w = 0;
            for (auto w : col_widths_) {
                acc_w += w;
                if (pt.x <= acc_w) {
                    break;
                }
                ++col;
            }
            if (row < rows_ && col < cols_) {
                return std::make_pair(row, col);
            }
        }
        return std::make_pair(-1, -1);
    }

  protected:
    std::vector<int> col_widths_;
    int rows_ = 0;
    int cols_ = 0;
    int row_pad_ = 0;
    int row_height_ = 0;
};

template <typename T>
struct GridLayoutContainer : public GridLayout {
    GridLayoutContainer() {
        items = std::vector<std::vector<T>>(cols_, std::vector<T>(rows_, T{}));
    }

    GridLayoutContainer(int num_rows, int num_cols, int min_col_width = 0) :
        GridLayout(num_rows, num_cols, min_col_width) {
        items = std::vector<std::vector<T>>(cols_, std::vector<T>(rows_, T{}));
    }

    void AddItem(int row, int col, T item) {
        assert(row <= rows_);
        assert(col <= cols_);
        items[col][row] = item;
    }

    T &GetItem(int row, int col) {
        assert(row <= rows_);
        assert(col <= cols_);
        return items[col][row];
    }

    T *GetItemByHit(Point pt) {
        auto [row, col] = HitTest(pt);
        if (row != -1 && col != -1) {
            return &items[col][row];
        }

        return nullptr;
    }

    std::vector<std::vector<T>> items;
};

} // namespace khiin::geometry
