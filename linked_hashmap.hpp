/**
 * implement a container like std::linked_hashmap
 */
#ifndef SJTU_LINKEDHASHMAP_HPP
#define SJTU_LINKEDHASHMAP_HPP

// only for std::equal_to<T> and std::hash<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<class Key, class T, class Hash = std::hash<Key>, class Equal = std::equal_to<Key>>
class linked_hashmap {
public:
	typedef pair<const Key, T> value_type;

private:
	struct Node {
		value_type data;
		Node* next_bucket;
		Node* prev_order;
		Node* next_order;
		linked_hashmap* container;

		Node(const Key& k, const T& v, Node* nb = nullptr, Node* po = nullptr, Node* no = nullptr, linked_hashmap* c = nullptr)
			: data(k, v), next_bucket(nb), prev_order(po), next_order(no), container(c) {}
	};

	Node** buckets;
	size_t bucket_count;
	size_t num_elements;
	Node* head;
	Node* tail;
	Hash hash_func;
	Equal equal_func;

	static const size_t DEFAULT_CAPACITY = 16;
	static const double LOAD_FACTOR;

	void init() {
		bucket_count = DEFAULT_CAPACITY;
		buckets = new Node*[bucket_count];
		for (size_t i = 0; i < bucket_count; ++i) {
			buckets[i] = nullptr;
		}
		num_elements = 0;
		head = nullptr;
		tail = nullptr;
	}

	void clear_all() {
		for (size_t i = 0; i < bucket_count; ++i) {
			Node* curr = buckets[i];
			while (curr != nullptr) {
				Node* next = curr->next_bucket;
				delete curr;
				curr = next;
			}
			buckets[i] = nullptr;
		}
		num_elements = 0;
		head = nullptr;
		tail = nullptr;
	}

	void rehash(size_t new_capacity) {
		Node** old_buckets = buckets;
		size_t old_count = bucket_count;

		bucket_count = new_capacity;
		buckets = new Node*[bucket_count];
		for (size_t i = 0; i < bucket_count; ++i) {
			buckets[i] = nullptr;
		}

		for (size_t i = 0; i < old_count; ++i) {
			Node* curr = old_buckets[i];
			while (curr != nullptr) {
				Node* next = curr->next_bucket;
				size_t index = hash_func(curr->data.first) % bucket_count;
				curr->next_bucket = buckets[index];
				buckets[index] = curr;
				curr = next;
			}
		}

		delete[] old_buckets;
	}

	void check_expand() {
		if (static_cast<double>(num_elements) / bucket_count >= LOAD_FACTOR) {
			rehash(bucket_count * 2);
		}
	}

public:
	class const_iterator;
	class iterator {
	private:
		Node* node;
		linked_hashmap* container;

	public:
		using difference_type = std::ptrdiff_t;
		using value_type = typename linked_hashmap::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::bidirectional_iterator_tag;

		iterator() : node(nullptr), container(nullptr) {}

		iterator(Node* n, linked_hashmap* c) : node(n), container(c) {}

		iterator(const iterator& other) : node(other.node), container(other.container) {}

		iterator operator++(int) {
			if (node == nullptr || container == nullptr) {
				throw invalid_iterator();
			}
			iterator tmp = *this;
			node = node->next_order;
			return tmp;
		}

		iterator& operator++() {
			if (node == nullptr || container == nullptr) {
				throw invalid_iterator();
			}
			node = node->next_order;
			return *this;
		}

		iterator operator--(int) {
			if (container == nullptr || (node == nullptr && container->head == nullptr)) {
				throw invalid_iterator();
			}
			iterator tmp = *this;
			if (node == nullptr) {
				node = container->tail;
			} else {
				node = node->prev_order;
			}
			return tmp;
		}

		iterator& operator--() {
			if (container == nullptr || (node == nullptr && container->head == nullptr)) {
				throw invalid_iterator();
			}
			if (node == nullptr) {
				node = container->tail;
			} else {
				node = node->prev_order;
			}
			return *this;
		}

		value_type& operator*() const {
			if (node == nullptr) {
				throw invalid_iterator();
			}
			return node->data;
		}

		bool operator==(const iterator& rhs) const {
			return node == rhs.node && container == rhs.container;
		}

		bool operator==(const const_iterator& rhs) const {
			return node == rhs.node && container == rhs.container;
		}

		bool operator!=(const iterator& rhs) const {
			return node != rhs.node || container != rhs.container;
		}

		bool operator!=(const const_iterator& rhs) const {
			return node != rhs.node || container != rhs.container;
		}

		value_type* operator->() const noexcept {
			return &(node->data);
		}

		friend class linked_hashmap;
		friend class const_iterator;
	};

	class const_iterator {
	private:
		const Node* node;
		const linked_hashmap* container;

	public:
		using difference_type = std::ptrdiff_t;
		using value_type = typename linked_hashmap::value_type;
		using pointer = const value_type*;
		using reference = const value_type&;
		using iterator_category = std::bidirectional_iterator_tag;

		const_iterator() : node(nullptr), container(nullptr) {}

		const_iterator(const Node* n, const linked_hashmap* c) : node(n), container(c) {}

		const_iterator(const const_iterator& other) : node(other.node), container(other.container) {}

		const_iterator(const iterator& other) : node(other.node), container(other.container) {}

		const_iterator operator++(int) {
			if (node == nullptr || container == nullptr) {
				throw invalid_iterator();
			}
			const_iterator tmp = *this;
			node = node->next_order;
			return tmp;
		}

		const_iterator& operator++() {
			if (node == nullptr || container == nullptr) {
				throw invalid_iterator();
			}
			node = node->next_order;
			return *this;
		}

		const_iterator operator--(int) {
			if (container == nullptr || (node == nullptr && container->head == nullptr)) {
				throw invalid_iterator();
			}
			const_iterator tmp = *this;
			if (node == nullptr) {
				node = container->tail;
			} else {
				node = node->prev_order;
			}
			return tmp;
		}

		const_iterator& operator--() {
			if (container == nullptr || (node == nullptr && container->head == nullptr)) {
				throw invalid_iterator();
			}
			if (node == nullptr) {
				node = container->tail;
			} else {
				node = node->prev_order;
			}
			return *this;
		}

		const value_type& operator*() const {
			if (node == nullptr) {
				throw invalid_iterator();
			}
			return node->data;
		}

		bool operator==(const iterator& rhs) const {
			return node == rhs.node && container == rhs.container;
		}

		bool operator==(const const_iterator& rhs) const {
			return node == rhs.node && container == rhs.container;
		}

		bool operator!=(const iterator& rhs) const {
			return node != rhs.node || container != rhs.container;
		}

		bool operator!=(const const_iterator& rhs) const {
			return node != rhs.node || container != rhs.container;
		}

		const value_type* operator->() const noexcept {
			return &(node->data);
		}

		friend class linked_hashmap;
	};

	linked_hashmap() {
		init();
	}

	linked_hashmap(const linked_hashmap& other) {
		init();
		for (const_iterator it = other.cbegin(); it != other.cend(); ++it) {
			insert(*it);
		}
	}

	linked_hashmap& operator=(const linked_hashmap& other) {
		if (this != &other) {
			clear_all();
			for (const_iterator it = other.cbegin(); it != other.cend(); ++it) {
				insert(*it);
			}
		}
		return *this;
	}

	~linked_hashmap() {
		clear_all();
		delete[] buckets;
	}

	T& at(const Key& key) {
		iterator it = find(key);
		if (it == end()) {
			throw index_out_of_bound();
		}
		return it->second;
	}

	const T& at(const Key& key) const {
		const_iterator it = find(key);
		if (it == cend()) {
			throw index_out_of_bound();
		}
		return it->second;
	}

	T& operator[](const Key& key) {
		iterator it = find(key);
		if (it != end()) {
			return it->second;
		}
		pair<iterator, bool> result = insert(value_type(key, T()));
		return result.first->second;
	}

	const T& operator[](const Key& key) const {
		return at(key);
	}

	iterator begin() {
		return iterator(head, this);
	}

	const_iterator cbegin() const {
		return const_iterator(head, this);
	}

	iterator end() {
		return iterator(nullptr, this);
	}

	const_iterator cend() const {
		return const_iterator(nullptr, this);
	}

	bool empty() const {
		return num_elements == 0;
	}

	size_t size() const {
		return num_elements;
	}

	void clear() {
		clear_all();
	}

	pair<iterator, bool> insert(const value_type& value) {
		iterator it = find(value.first);
		if (it != end()) {
			return pair<iterator, bool>(it, false);
		}

		check_expand();

		size_t index = hash_func(value.first) % bucket_count;
		Node* new_node = new Node(value.first, value.second, buckets[index], tail, nullptr, this);
		buckets[index] = new_node;

		if (tail == nullptr) {
			head = new_node;
		} else {
			tail->next_order = new_node;
		}
		tail = new_node;

		num_elements++;
		return pair<iterator, bool>(iterator(new_node, this), true);
	}

	void erase(iterator pos) {
		if (pos == end() || pos.container != this) {
			throw invalid_iterator();
		}

		Node* node_to_erase = pos.node;

		size_t index = hash_func(node_to_erase->data.first) % bucket_count;
		Node** curr = &buckets[index];
		while (*curr != nullptr) {
			if (*curr == node_to_erase) {
				*curr = (*curr)->next_bucket;
				break;
			}
			curr = &((*curr)->next_bucket);
		}

		if (node_to_erase->prev_order != nullptr) {
			node_to_erase->prev_order->next_order = node_to_erase->next_order;
		} else {
			head = node_to_erase->next_order;
		}

		if (node_to_erase->next_order != nullptr) {
			node_to_erase->next_order->prev_order = node_to_erase->prev_order;
		} else {
			tail = node_to_erase->prev_order;
		}

		delete node_to_erase;
		num_elements--;
	}

	size_t count(const Key& key) const {
		return find(key) == cend() ? 0 : 1;
	}

	iterator find(const Key& key) {
		size_t index = hash_func(key) % bucket_count;
		Node* curr = buckets[index];
		while (curr != nullptr) {
			if (equal_func(curr->data.first, key)) {
				return iterator(curr, this);
			}
			curr = curr->next_bucket;
		}
		return end();
	}

	const_iterator find(const Key& key) const {
		size_t index = hash_func(key) % bucket_count;
		Node* curr = buckets[index];
		while (curr != nullptr) {
			if (equal_func(curr->data.first, key)) {
				return const_iterator(curr, this);
			}
			curr = curr->next_bucket;
		}
		return cend();
	}
};

template<class Key, class T, class Hash, class Equal>
const double linked_hashmap<Key, T, Hash, Equal>::LOAD_FACTOR = 0.75;

}

#endif
