#include "scanline_triangulation.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace geometry {

    std::unique_ptr<DelaunayBuilder> DelaunayBuilder::Create(
        std::vector<Vector2D> points) {
        DelaunayBuilder builder(std::move(points));
        builder.Build();
        return std::make_unique<DelaunayBuilder>(std::move(builder));
    }

    bool DelaunayBuilder::CheckDelaunayCondition(
        int left, int right, int outer, int inner) const {
        const auto& l = triangulation_.points[left];
        const auto& r = triangulation_.points[right];
        const auto& t /*top*/ = triangulation_.points[outer];
        const auto& b /*bottom*/ = triangulation_.points[inner];

        // �������� �� ��, ��� ����� ���������������
        if (outer == inner) {
            return true;
        }

        // �������� �� ����������
        if (CrossProduct(l - t, b - t) < 0 || CrossProduct(r - t, b - t) > 0) {
            return true;
        }

        // �������� ������� ������, ��� � ����� �� ������
        const auto Sa = (t.x - r.x) * (t.x - l.x) + (t.y - r.y) * (t.y - l.y);
        const auto Sb = (b.x - r.x) * (b.x - l.x) + (b.y - r.y) * (b.y - l.y);
        if (Sa > -eps && Sb > -eps) {
            return true;
        }
        if (!(Sa < 0 && Sb < 0)) {
            auto Sc = CrossProduct(t - r, t - l);
            auto Sd = CrossProduct(b - r, b - l);
            if (Sc < 0) Sc = -Sc;
            if (Sd < 0) Sd = -Sd;
            if (Sc * Sb + Sa * Sd > -eps) {
                return true;
            }
        }
        return false;
    }

    // outer ������ ����������� �����, left � right - ������� � ��� � ������������
    // ��������� ����� ��� ���������������� inner ������ �� ����� {left, right}
    void DelaunayBuilder::FixTriangulation(int left, int right, int outer) {
        auto& graph_ = triangulation_.graph;

        // ������������� ����� ��������
        // ������� ���������� left � right
        recursion_stack_[0] = Edge{ left, right };
        int stack_size = 1;
        while (stack_size > 0) {
            left = recursion_stack_[stack_size - 1].v1;
            right = recursion_stack_[stack_size - 1].v2;
            --stack_size;

            // ����� ������� � ���������, ������ ��� outer > ������� ����� ����������� �����
            int inner =
                graph_[Edge{ std::min(left, right), std::max(left, right) }].Min();
            if (CheckDelaunayCondition(left, right, outer, inner)) {
                // ���� ������ ������ � ���������������� �� ����,
                // ������ ��������� ����������� ����� � �������
                graph_[Edge{ right, outer }].Insert(left);
                graph_[Edge{ left, outer }].Insert(right);
                if (right < left) std::swap(right, left);
                graph_[Edge{ left, right }].Insert(outer);
                continue;
            }

            // ����� ������������� ������������ � ����������������
            graph_[Edge{ right, outer }].Replace(left, inner);
            graph_[Edge{ left, outer }].Replace(right, inner);
            graph_[Edge{ std::min(inner, left), std::max(inner, left) }].Replace(right, outer);
            graph_[Edge{ std::min(inner, right), std::max(inner, right) }].Replace(left, outer);

            graph_.erase(Edge{ std::min(left, right), std::max(left, right) });

            // � ��������� 2 ����� ����������� ������
            recursion_stack_[stack_size++] = Edge{ left, inner };
            recursion_stack_[stack_size++] = Edge{ inner, right };
        }
    }

    void DelaunayBuilder::AddPointToTriangulation(int i) {
        auto& points_ = triangulation_.points;

        int hull_pt = i - 1;
        auto last_vector = points_[hull_pt] - points_[i];
        int next_hull_pt = convex_hull_[hull_pt].right;
        auto new_vector = points_[next_hull_pt] - points_[i];

        // ����, �������� ������ �� �����, ����� �������, ��������� ����������� ������������
        while (CrossProduct(last_vector, new_vector) > -eps) {
            FixTriangulation(hull_pt, next_hull_pt, i);

            hull_pt = next_hull_pt;
            last_vector = new_vector;
            next_hull_pt = convex_hull_[hull_pt].right;
            new_vector = points_[next_hull_pt] - points_[i];
        }
        convex_hull_[i].right = hull_pt;

        // ���������� ��� �������� �����, ���� ����� �������
        hull_pt = i - 1;
        last_vector = points_[hull_pt] - points_[i];
        next_hull_pt = convex_hull_[hull_pt].left;
        new_vector = points_[next_hull_pt] - points_[i];

        while (CrossProduct(last_vector, new_vector) < eps) {
            FixTriangulation(next_hull_pt, hull_pt, i);

            hull_pt = next_hull_pt;
            last_vector = new_vector;
            next_hull_pt = convex_hull_[hull_pt].left;
            new_vector = points_[next_hull_pt] - points_[i];
        }
        convex_hull_[i].left = hull_pt;

        convex_hull_[convex_hull_[i].right].left = i;
        convex_hull_[hull_pt].right = i;
    }

    void DelaunayBuilder::Build() {
        auto& graph_ = triangulation_.graph;
        auto& points_ = triangulation_.points;

        // ������ ����� �������� � ��� ������� �� ��������� ���������� �����
        recursion_stack_.resize(points_.size());
        convex_hull_.resize(points_.size());

        std::sort(points_.begin(), points_.end(), [](auto& lhs, auto& rhs) {
            return lhs.x < rhs.x;
            });

        if (points_.size() < 3) {
            return;  // ���� ����� ���� �����, �� ������������ ������
        }

        // ������������� ������� ������������
        convex_hull_[0] = ListNode{ 1, 1 };
        convex_hull_[1] = ListNode{ 0, 0 };
        graph_[Edge{ 0, 1 }].Insert(2);

        for (int i = 2; i < points_.size(); ++i) {
            AddPointToTriangulation(i);
        }
        // ��� � triangulation ��������� ��� ������������ � ������� ����� -> ��� �������
        // ������ ����� � ��� ������ ���, ��� ������
    }
}  // namespace geometry