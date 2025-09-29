#include "container.hpp"

#include <span>

namespace ae {

container::container(std::span<const element_type> data) {
  for (auto first = data.begin(); first < data.end(); ++first) {
      placeholder_.emplace_back(*first);
  }
}

std::size_t container::size() const {
    return placeholder_.size();
}

container::element_type& container::operator[](std::size_t idx) {
	return placeholder_[idx];
}


}  // namespace ae
