//
//
// Copyright (C) 2019 Tencent. All rights reserved.
// Flare is licensed under the BSD 3-Clause License.
// The source codes in this file based on
// https://github.com/Tencent/flare/blob/master/flare/base/internal/doubly_linked_list.h.
// This source file may have been modified by Tencent, and licensed under the BSD 3-Clause License.
//
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

#include "trpc/util/check.h"
#include "trpc/util/likely.h"

namespace trpc {

struct DoublyLinkedListEntry {
  DoublyLinkedListEntry* prev = this;
  DoublyLinkedListEntry* next = this;
};

/// @brief Non-owning doubly linked list. Thread-compatible.
/// @note For internal use only. DO NOT USE IT.
//        The interface does not quite match STL's, we might want to align with them.
template <class T, DoublyLinkedListEntry T::*kEntry>
class DoublyLinkedList {
 public:
  class iterator;
  class const_iterator;

  // Initialize an empty list.
  constexpr DoublyLinkedList();
  ~DoublyLinkedList();

  /// @brief Access first element in the list.
  /// @return `T` reference
  /// @note  Precondition: `!empty()` holds.
  constexpr T& front() noexcept;
  constexpr const T& front() const noexcept;

  /// @brief Access last element in the list.
  /// @note Precondition: `!empty()` holds.
  constexpr T& back() noexcept;
  constexpr const T& back() const noexcept;

  /// @brief Pop the first element (at head) in the list, or `nullptr` if the list is empty.
  /// @return `T*` pointer
  constexpr T* pop_front() noexcept;

  /// @brief Pop the last element (at the tail) in the list, or `nullptr` if the list is empty.
  constexpr T* pop_back() noexcept;

  /// @brief Insert an element at list's head.
  /// @param entry element of `T*` pointer
  constexpr void push_front(T* entry) noexcept;

  /// @brief Push an element into the list. The element is inserted at the tail of the list.
  /// @param entry element of `T*` pointer
  constexpr void push_back(T* entry) noexcept;

  /// @brief After `erase`, the list node is initialized to `{this, this}`. This is
  ///        needed to detect if the node is not in the list.
  /// @param entry element of `T*` pointer
  /// @return Returns `true` if `entry` was successfully removed, `false` if it's not in
  ///         the list (i.e., both pointer point to itself).
  constexpr bool erase(T* entry) noexcept;

  /// @brief Move all elements from `from` to the tail of this list.
  /// @param from other list
  constexpr void splice(DoublyLinkedList&& from) noexcept;

  /// @brief Swap two lists.
  /// @param other other list
  constexpr void swap(DoublyLinkedList& other) noexcept;

  /// @brief Get size of the list.
  /// @return std::size_t size of this list
  constexpr std::size_t size() const noexcept;

  /// @brief Test if the list is empty.
  /// @return bool true: empty; false: not empty
  constexpr bool empty() const noexcept;

  /// @brief Iterate through the list.
  constexpr iterator begin() noexcept;
  constexpr iterator end() noexcept;
  constexpr const_iterator begin() const noexcept;
  constexpr const_iterator end() const noexcept;

  // Non-movable, non-copyable
  DoublyLinkedList(const DoublyLinkedList&) = delete;
  DoublyLinkedList& operator=(const DoublyLinkedList&) = delete;

 private:
  // `offsetof(T, kEntry)`.
  inline static const std::uintptr_t kDoublyLinkedListEntryOffset =
      reinterpret_cast<std::uintptr_t>(&(reinterpret_cast<T*>(0)->*kEntry));

  // Cast from `T*` to pointer to its `DoublyLinkedListEntry` field.
  static constexpr DoublyLinkedListEntry* node_cast(T* ptr) noexcept;
  static constexpr const DoublyLinkedListEntry* node_cast(const T* ptr) noexcept;

  // Cast back from what's returned by `node_cast` to original `T*`.
  static constexpr T* object_cast(DoublyLinkedListEntry* entry) noexcept;
  static constexpr const T* object_cast(const DoublyLinkedListEntry* entry) noexcept;

 private:
  std::size_t size_{};
  DoublyLinkedListEntry head_;
};

// Mutable iterator.
template <class T, DoublyLinkedListEntry T::*kEntry>
class DoublyLinkedList<T, kEntry>::iterator {
 public:
  constexpr iterator() : current_(nullptr) {}
  constexpr iterator& operator++() noexcept {
    current_ = current_->next;
    return *this;
  }
  constexpr T* operator->() const noexcept { return object_cast(current_); }
  constexpr T& operator*() const noexcept { return *object_cast(current_); }
  constexpr bool operator!=(const iterator& iter) const noexcept { return current_ != iter.current_; }

  // Not support Post-increment / decrement / operator ==.
 private:
  friend class DoublyLinkedList<T, kEntry>;
  constexpr explicit iterator(DoublyLinkedListEntry* start) noexcept : current_(start) {}

 private:
  DoublyLinkedListEntry* current_;
};

// Const iterator.
template <class T, DoublyLinkedListEntry T::*kEntry>
class DoublyLinkedList<T, kEntry>::const_iterator {
 public:
  constexpr const_iterator() : current_(nullptr) {}
  constexpr /* implicit */ const_iterator(const iterator& from) : current_(from.current_) {}
  constexpr const_iterator& operator++() noexcept {
    current_ = current_->next;
    return *this;
  }
  constexpr const T* operator->() const noexcept { return object_cast(current_); }
  constexpr const T& operator*() const noexcept { return *object_cast(current_); }
  constexpr bool operator!=(const const_iterator& iter) const noexcept { return current_ != iter.current_; }

  // Not support Post-increment / decrement / operator ==.
 private:
  friend class DoublyLinkedList<T, kEntry>;
  constexpr explicit const_iterator(const DoublyLinkedListEntry* start) noexcept : current_(start) {}

 private:
  const DoublyLinkedListEntry* current_;
};

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr inline DoublyLinkedList<T, kEntry>::DoublyLinkedList() = default;

template <class T, DoublyLinkedListEntry T::*kEntry>
DoublyLinkedList<T, kEntry>::~DoublyLinkedList() {
  while (!empty()) {
    pop_front();
  }
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr T& DoublyLinkedList<T, kEntry>::front() noexcept {
  TRPC_DCHECK_NE(head_.next, &head_, "Calling `front()` on empty list is undefined.");
  return *object_cast(head_.next);
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr const T& DoublyLinkedList<T, kEntry>::front() const noexcept {
  return const_cast<DoublyLinkedList*>(this)->front();
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr T& DoublyLinkedList<T, kEntry>::back() noexcept {
  TRPC_DCHECK_NE(head_.prev, &head_, "Calling `back()` on empty list is undefined.");
  return *object_cast(head_.prev);
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr const T& DoublyLinkedList<T, kEntry>::back() const noexcept {
  return const_cast<DoublyLinkedList*>(this)->back();
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr T* DoublyLinkedList<T, kEntry>::pop_front() noexcept {
  if (TRPC_UNLIKELY(head_.prev == &head_)) {
    // Empty then.
    TRPC_DCHECK_EQ(head_.prev, head_.next);
    return nullptr;
  }
  auto rc = head_.next;
  rc->prev->next = rc->next;
  rc->next->prev = rc->prev;
  rc->next = rc->prev = rc;  // Mark it as removed.
  --size_;
  return object_cast(rc);
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr T* DoublyLinkedList<T, kEntry>::pop_back() noexcept {
  if (TRPC_UNLIKELY(head_.prev == &head_)) {
    // Empty then.
    TRPC_DCHECK_EQ(head_.prev, head_.next);
    return nullptr;
  }
  auto rc = head_.prev;
  rc->prev->next = rc->next;
  rc->next->prev = rc->prev;
  rc->next = rc->prev = rc;  // Mark it as removed.
  --size_;
  return object_cast(rc);
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr void DoublyLinkedList<T, kEntry>::push_front(T* entry) noexcept {
  auto* ptr = node_cast(entry);
  TRPC_DCHECK_EQ(ptr->prev, ptr->next, "push_front `entry` is already in other list");
  TRPC_DCHECK_EQ(ptr, ptr->next, "push_front `entry` is already in other list");
  ptr->prev = &head_;
  ptr->next = head_.next;
  // Add the node at the front.
  ptr->prev->next = ptr;
  ptr->next->prev = ptr;
  ++size_;
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr void DoublyLinkedList<T, kEntry>::push_back(T* entry) noexcept {
  auto* ptr = node_cast(entry);
  TRPC_DCHECK_EQ(ptr->prev, ptr->next, "push_back `entry` is already in other list");
  TRPC_DCHECK_EQ(ptr, ptr->next, "push_back `entry` is already in other list");
  ptr->prev = head_.prev;
  ptr->next = &head_;
  // Add the node to the tail.
  ptr->prev->next = ptr;
  ptr->next->prev = ptr;
  ++size_;
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr bool DoublyLinkedList<T, kEntry>::erase(T* entry) noexcept {
  auto* ptr = node_cast(entry);
  // Ensure that we're indeed in the list.
  if (ptr->prev == ptr) {
    TRPC_DCHECK_EQ(ptr->prev, ptr->next);
    return false;
  }
  ptr->prev->next = ptr->next;
  ptr->next->prev = ptr->prev;
  ptr->next = ptr->prev = ptr;
  --size_;
  return true;
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr void DoublyLinkedList<T, kEntry>::splice(DoublyLinkedList&& from) noexcept {
  if (from.empty()) {
    return;
  }
  auto&& other_front = from.head_.next;
  auto&& other_back = from.head_.prev;
  // Link `from`'s head with our tail.
  other_front->prev = head_.prev;
  head_.prev->next = other_front;
  // Link `from`'s tail with `head_`.
  other_back->next = &head_;
  head_.prev = other_back;
  // Update size.
  size_ += std::exchange(from.size_, 0);
  // Clear `from`.
  from.head_.prev = from.head_.next = &from.head_;
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr void DoublyLinkedList<T, kEntry>::swap(DoublyLinkedList& other) noexcept {
  bool e1 = empty(), e2 = other.empty();
  std::swap(head_, other.head_);
  std::swap(size_, other.size_);
  if (!e2) {
    head_.prev->next = &head_;
    head_.next->prev = &head_;
  } else {
    head_.prev = head_.next = &head_;
  }
  if (!e1) {
    other.head_.prev->next = &other.head_;
    other.head_.next->prev = &other.head_;
  } else {
    other.head_.prev = other.head_.next = &other.head_;
  }
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr std::size_t DoublyLinkedList<T, kEntry>::size() const noexcept {
  return size_;
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr bool DoublyLinkedList<T, kEntry>::empty() const noexcept {
  TRPC_DCHECK(!size_ == (head_.prev == &head_));
  return !size_;
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr typename DoublyLinkedList<T, kEntry>::iterator DoublyLinkedList<T, kEntry>::begin() noexcept {
  return iterator(head_.next);
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr typename DoublyLinkedList<T, kEntry>::iterator DoublyLinkedList<T, kEntry>::end() noexcept {
  return iterator(&head_);
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr typename DoublyLinkedList<T, kEntry>::const_iterator DoublyLinkedList<T, kEntry>::begin() const noexcept {
  return const_iterator(head_.next);
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr typename DoublyLinkedList<T, kEntry>::const_iterator DoublyLinkedList<T, kEntry>::end() const noexcept {
  return const_iterator(&head_);
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr DoublyLinkedListEntry* DoublyLinkedList<T, kEntry>::node_cast(T* ptr) noexcept {
  return &(ptr->*kEntry);
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr const DoublyLinkedListEntry* DoublyLinkedList<T, kEntry>::node_cast(const T* ptr) noexcept {
  return &(ptr->*kEntry);
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr T* DoublyLinkedList<T, kEntry>::object_cast(DoublyLinkedListEntry* entry) noexcept {
  return reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(entry) - kDoublyLinkedListEntryOffset);
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr const T* DoublyLinkedList<T, kEntry>::object_cast(const DoublyLinkedListEntry* entry) noexcept {
  return reinterpret_cast<const T*>(reinterpret_cast<std::uintptr_t>(entry) - kDoublyLinkedListEntryOffset);
}

template <class T, DoublyLinkedListEntry T::*kEntry>
constexpr void swap(DoublyLinkedList<T, kEntry>& left, DoublyLinkedList<T, kEntry>& right) noexcept {
  left.swap(right);
}

}  // namespace trpc
