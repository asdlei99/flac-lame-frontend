/**
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * Copyright 2017-present Rodrigo Cesar de Freitas Dias
 * This library is released under the MIT License
 */

#pragma once
#include <vector>

namespace wl {

// Vector-based associative container which holds the insertion order.
template<typename keyT, typename valueT>
class held_map final {
public:
	struct entry final {
		keyT   key;
		valueT value;
		entry() = default;
		explicit entry(const keyT& key) noexcept : key(key) { }
	};

private:
	std::vector<entry> _entries;

public:
	held_map() = default;
	held_map(std::initializer_list<entry> entries) noexcept : _entries{entries} { }

	size_t    size() const noexcept               { return this->_entries.size(); }
	bool      empty() const noexcept              { return this->_entries.empty(); }
	held_map& clear() noexcept                    { this->_entries.clear(); return *this; }
	held_map& reserve(size_t numEntries) noexcept { this->_entries.reserve(numEntries); return *this; }

	const valueT& operator[](const keyT& key) const {
		std::vector<entry>::const_iterator ite = this->_find(key);
		if (ite == this->_entries.cend()) {
			throw std::out_of_range("Key doesn't exist.");
		}
		return ite->value;
	}

	valueT& operator[](const keyT& key) noexcept {
		std::vector<entry>::iterator ite = this->_find(key);
		if (ite == this->_entries.end()) {
			this->_entries.emplace_back(key); // inexistent, so add
			return this->_entries.back().value;
		}
		return ite->value;
	}

	// Returns pointer to value, if key doesn't exist returns nullptr.
	const valueT* get_if_exists(const keyT& key) const noexcept {
		// Saves time, instead of calling has() and operator[]().
		std::vector<entry>::const_iterator ite = this->_find(key);
		return (ite == this->_entries.cend()) ?
			nullptr : &ite->value;
	}

	// Returns pointer to value, if key doesn't exist returns nullptr.
	valueT* get_if_exists(const keyT& key) noexcept {
		std::vector<entry>::iterator ite = this->_find(key);
		return (ite == this->_entries.end()) ?
			nullptr : &ite->value;
	}

	// Does the key exist?
	bool has(const keyT& key) const noexcept {
		return this->_find(key) != this->_entries.cend();
	}

	held_map& remove(const keyT& key) noexcept {
		std::vector<entry>::iterator ite = this->_find(key);
		if (ite != this->_entries.end()) { // won't fail if inexistent
			this->_entries.erase(ite);
		}
		return *this;
	}

private:
	typename std::vector<entry>::const_iterator _find(const keyT& key) const noexcept {
		for (std::vector<entry>::const_iterator ite = this->_entries.cbegin();
			ite != this->_entries.cend(); ++ite)
		{
			if (ite->key == key) return ite;
		}
		return this->_entries.cend();
	}

	typename std::vector<entry>::iterator _find(const keyT& key) noexcept {
		for (std::vector<entry>::iterator ite = this->_entries.begin();
			ite != this->_entries.end(); ++ite)
		{
			if (ite->key == key) return ite;
		}
		return this->_entries.end();
	}

public:
	class const_iterator final {
	private:
		typename std::vector<entry>::const_iterator _it;
	public:
		const_iterator(typename std::vector<entry>::const_iterator it) noexcept : _it(it) { }
		const_iterator operator++() noexcept                                  { const_iterator tmp = *this; this->_it++; return tmp; }
		const_iterator operator++(int) noexcept                               { this->_it++; return *this; }
		const entry&   operator*() noexcept                                   { return this->_it.operator*(); }
		const entry*   operator->() noexcept                                  { return this->_it.operator->(); }
		bool           operator==(const const_iterator& other) const noexcept { return this->_it == other._it; }
		bool           operator!=(const const_iterator& other) const noexcept { return !this->operator==(other); }
	};

	class iterator final {
	private:
		typename std::vector<entry>::iterator _it;
	public:
		iterator(typename std::vector<entry>::iterator it) noexcept : _it(it) { }
		iterator operator++() noexcept                            { iterator tmp = *this; this->_it++; return tmp; }
		iterator operator++(int) noexcept                         { this->_it++; return *this; }
		entry&   operator*() noexcept                             { return this->_it.operator*(); }
		entry*   operator->() noexcept                            { return this->_it.operator->(); }
		bool     operator==(const iterator& other) const noexcept { return this->_it == other._it; }
		bool     operator!=(const iterator& other) const noexcept { return !this->operator==(other); }
	};

	const_iterator cbegin() const noexcept { return {this->_entries.cbegin()}; }
	const_iterator begin() const noexcept  { return {this->_entries.cbegin()}; }
	iterator       begin() noexcept        { return {this->_entries.begin()}; }
	const_iterator cend() const noexcept   { return {this->_entries.cend()}; }
	const_iterator end() const noexcept    { return {this->_entries.cend()}; }
	iterator       end() noexcept          { return {this->_entries.end()}; }
};

}//namespace wl
