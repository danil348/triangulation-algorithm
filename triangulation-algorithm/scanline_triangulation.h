#pragma once

#include "models.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace geometry {

    // ������ ����������� ���������� ������ �������, �� � ������ ����� ��������� ��������
    const auto eps = 1e-9;

    class DelaunayBuilder {

    public:
        DelaunayBuilder(const DelaunayBuilder&) = delete;
        const DelaunayBuilder& operator=(const DelaunayBuilder&) = delete;
        DelaunayBuilder(DelaunayBuilder&&) = default;
        DelaunayBuilder& operator=(DelaunayBuilder&&) = default;

        inline const DelaunayTriangulation& Get() const { return triangulation_; }

        static std::unique_ptr<DelaunayBuilder> Create(std::vector<Vector2D> points);

        // ���������, ����� �� ������������� ���������������, �������� ��������� ����� � ����������
        bool CheckDelaunayCondition(int left, int right, int outer, int inner) const;
    private:
        explicit DelaunayBuilder(std::vector<Vector2D> points) noexcept {
            triangulation_.points = std::move(points);
        }

        // ������ ������������
        void Build();

        void AddPointToTriangulation(int i /*index*/);

        // �������, ������� ���������� ������������� ������������ (��� 4)
        void FixTriangulation(int left, int right, int outer);


        // ���������� �������� � ������� �����, ����� �� ���� ������������
        std::vector<Edge> recursion_stack_{};

        // ��������� ��� �������� ������������: ����� -> ���� ������ + �����
        DelaunayTriangulation triangulation_{};

        List convex_hull_{};
    };
}