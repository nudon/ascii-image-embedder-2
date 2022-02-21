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
  double Comparer<Data>::Compare(Data a, Data b) {
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
  bool DataMatch<D,I>::better_match(Entry *other) {
    bool ret = false;
    if (!has_match()) {
      ret = true;
    }
    else {
      //ah no acess Data in DME
      ret = comparer->Compare(base->get_data(), other->get_data()) < match_score;
      if (!comparer->compares_as_distance()) {
	ret = !ret;
      }
    }
    return ret;
  }

  template<class D, class I>
  void DataMatch<D,I>::set_match(Entry *other) {
    double val = comparer->Compare(base->get_data(), other->get_data());
    match_score = val; 
    match = other;
  }
  
  template<class D, class I>
  void DataMatch<D,I>::reset_match() {
    match_score.reset();
    match = nullptr;
  }

  template<class D, class I>
  void DataMatch<D,I>::find_best_matches(MatchList &targets,EntryList &pool) {
    for(Match &target : targets) {
      for(Entry &entry : pool) {
	if (target.better_match(&entry)) {
	  target.set_match(&entry);
	}
      }
    }
  }
  
  template<class D, class I>
  void DataMatch<D,I>::find_best_match(EntryList &list) {
  }
}

#endif
