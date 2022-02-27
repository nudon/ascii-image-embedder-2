#ifndef FILE_MATCH_CPP_SEEN
#define FILE_MATCH_CPP_SEEN

#include <stdexcept>
#include "match.hpp"

namespace match {
  
  template<class Data>
  Comparer<Data>::Comparer(CompareFunc func, bool b) {
    if (func == nullptr) {
      throw std::invalid_argument("received null function pointer");
    }
    compare = func;
    compared_as_distance = b;
  }
  
  template<class Data>
  double Comparer<Data>::Compare(Data a,  Data b) {
    return compare(a,b);
  }

  template<class Data>
  bool Comparer<Data>::compares_as_distance() {
    return compared_as_distance;
  }

  template<class Data, class Index>
  DataMatchEntry<Data, Index>::DataMatchEntry(Data d, Index i) {
    data = d;
    index = i;
  }

  template<class Data, class Index>
  Data DataMatchEntry<Data, Index>::get_data() {
    return data;
  }

  template<class Data, class Index>
  Index DataMatchEntry<Data, Index>::get_index() {
    return index;
  }
  
  template<class D, class I>
  DataMatch<D,I>::DataMatch(Entry *entry, Comparer<D> *compare) {
    base = entry;
    comparer = compare;
  }

  template<class D, class I>
  DataMatchEntry<D,I>* DataMatch<D,I>::get_base() {
    return base;
  }

  template<class D, class I>
  DataMatchEntry<D,I>* DataMatch<D,I>::get_match() {
    return match;
  }  

  template<class D, class I>
  bool DataMatch<D,I>::has_match() {
    return match_score.has_value();
  }

  template<class D, class I>
  double DataMatch<D,I>::compare(Entry *other) {
    auto base_data = base->get_data();
    auto other_data = other->get_data();
    return comparer->Compare(base_data, other_data);
  }

  template<class D, class I>
  bool DataMatch<D,I>::better_match(Entry *other) {
    bool ret = false;
    if (!has_match()) {
      ret = true;
    }
    else {
      ret = compare(other) < match_score;
      if (!comparer->compares_as_distance()) {
	ret = !ret;
      }
    }
    return ret;
  }

  template<class D, class I>
  void DataMatch<D,I>::set_match(Entry *other) {
    double val = compare(other);
    match_score = val; 
    match = other;
  }
  
  template<class D, class I>
  void DataMatch<D,I>::reset_match() {
    match_score.reset();
    match = nullptr;
  }

  template<class D, class I>
  void DataMatch<D,I>::find_best_match(EntryList &pool) {
    for(Entry &entry : pool) {
      if (this->better_match(&entry)) {
	this->set_match(&entry);
      }
    }
  }

  template<class D,class I>
  auto ScoreMatcher<D,I>::build_entry_list(std::list< I> *i_list, std::list< D> *d_list) -> std::list<EntryT> {
    bool done = false;
    auto i_itr = i_list->begin();
    auto d_itr = d_list->begin();
    if (d_list->size() != i_list->size()) {
      throw std::runtime_error("Lists are not of the same size");
    }
    std::list<EntryT> pair_list;
    while(!done) {
      if (i_itr == i_list->end()) {
	done = true;
      }
      else {
	I &i = *i_itr;
	D &d = *d_itr;
	pair_list.emplace_back(&d, &i);
	i_itr++;
	d_itr++;
      }
    }
    return pair_list;
  }

  template<class D,class I> 
  auto ScoreMatcher<D,I>::build_match_list(std::list<EntryT> *entry_list, Comparer<D*> *comp) -> std::list<MatchT> {
    std::list<typename ScoreMatcher<D,I>::MatchT> match_list;
    for (auto &entry : *entry_list) {
      match_list.emplace_back(&entry, comp);
    }
    return match_list;
  }  
  
}

#endif
