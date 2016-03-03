#pragma once

#include <array>
#include <vector>
#include <string>

class grid_collection {
	public:
		grid_collection(const std::string& name, const std::array<double, 3>& origin, const std::array<double, 3>& delta, const std::array<int, 3>& iorigin);

		std::array<double, 3> scale() const;
		std::array<double, 3> origin() const;
		std::array<int, 3> iorigin() const;

		std::string name() const;

		bool isConsistentWith(const grid_collection& gc) const;

		grid_collection operator +(const grid_collection& gc) const;

	protected:
	private:
		std::array<double, 3> m_origin, m_delta;
		std::array<int, 3> m_iorigin;

		std::vector<std::string> m_nameBits;

	friend std::ostream& operator << (std::ostream& out, const grid_collection& gc);
};

std::ostream& operator << (std::ostream& out, const grid_collection& gc);
