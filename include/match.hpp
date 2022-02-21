#ifndef FILE_MATCH_SEEN
#define FILE_MATCH_SEEN

#include <list>
#include <optional>
namespace match {

  /*
    A template system for finding best matches within sets of data

    Has Comparer, which compares between Data 
    Has DataMatchEntry, which just pairs some processing data with some idea of it's source called Index
    Method definitions are within match.cpp, but instead of forward declaring template instatiations or moving all the method defitions here the cpp file is merely included at the end of this. 
   */

  /*
    compared_as_distance describes if the comparision function 
    returns the difference or similarity between data
    if true, lower numbers mean better matches, because there's less difference
    if false, higher numbers mean better matches, because there's more similarity
   */
  template<class Data>
  class Comparer {
    using CompareFunc = double (*)(Data a, Data b);
  public:
    Comparer(CompareFunc func, bool b);
    double Compare(Data a, Data b);
    bool compares_as_distance();
  private:
    bool compared_as_distance = true;
    CompareFunc compare = nullptr;
  };
  
  /*
    expects data to hold some processing data originating from an Index
    expects Data to have a double compare(Data& other) function
   */
  template<class Data, class Index>
  class DataMatchEntry {
  public:
    DataMatchEntry(Data d, Index i);
    Data get_data();
    Index get_index();
  private:
    Data data;
    Index index;
  };


  template<class Data, class Index>
  class DataMatch {
    using Entry = DataMatchEntry<Data, Index>;
    using Match = DataMatch<Data,Index>;
    using EntryList = std::list<Entry>;
    using MatchList = std::list<Match>;
  public:
    DataMatch(Entry *base_entry, Comparer<Data> *compare);
    Entry* get_base();
    Entry* get_match();
    bool has_match();
    bool better_match(Entry *other);
    void set_match(Entry *other);
    void reset_match();
    static void find_best_matches(MatchList &a, EntryList &b);
    void find_best_match(EntryList &list);
  private:
    Entry *base;
    Entry *match;
    Comparer<Data> *comparer = nullptr;
    std::optional<double> match_score;
    };
}
#include "match.cpp"
#endif
