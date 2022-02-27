#ifndef FILE_MATCH_SEEN
#define FILE_MATCH_SEEN

#include <list>
#include <optional>
namespace match {

  /*
    A template system for finding best matches within sets of data
    Method definitions are within match.cpp, but instead of forward declaring template instatiations or moving all the method defitions here the cpp file is merely included at the end of this. 
    
  */

  
  /*
    Compares data, compared_as_distance describes if it returns a difference or similarity measurement
    Ultimately desribing if lower or higher numbers mean a better match
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

    DataMatchEntry just pairs some processing data with some idea of it's source called Index
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


  /*
    DataMatch holds information on a base DataEntry
    storing a current score and also a pointer to closest match
   */
  template<class Data, class Index>
  class DataMatch {
  public:
    using Entry = DataMatchEntry<Data, Index>;
    using Match = DataMatch<Data, Index>;
    using EntryList = std::list<Entry>;
    using MatchList = std::list<Match>;
    DataMatch(Entry *base_entry, Comparer<Data> *compare);
    Entry* get_base();
    Entry* get_match();
    bool has_match();
    double compare(Entry* other);
    bool better_match(Entry *other);
    void set_match(Entry *other);
    void reset_match();
    void find_best_match(EntryList &list);
  private:
    Entry *base;
    Entry *match;
    Comparer<Data> *comparer = nullptr;
    std::optional<double> match_score;
  };

  /*
    Just used to help convert things into DataMatches and entries
   */
  template<class Data, class Index>
  class ScoreMatcher {
  public:
    using MatchT = match::DataMatch<Data*,Index*>;
    using EntryT = match::DataMatchEntry<Data*,Index*>;
    static std::list<EntryT> build_entry_list(std::list<Index> *i_list, std::list<Data> *d_list);
    static std::list<MatchT> build_match_list(std::list<EntryT> *entry_list, match::Comparer<Data*> *comp);
  private:
  };
}
#include "match.cpp"
#endif
