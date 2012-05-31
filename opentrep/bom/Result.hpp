#ifndef __OPENTREP_BOM_RESULT_HPP
#define __OPENTREP_BOM_RESULT_HPP

// //////////////////////////////////////////////////////////////////////
// Import section
// //////////////////////////////////////////////////////////////////////
// STL
#include <list>
#include <map>
// Xapian
#include <xapian.h>
// OpenTREP
#include <opentrep/OPENTREP_Types.hpp>
#include <opentrep/bom/BomAbstract.hpp>
#include <opentrep/bom/ScoreBoard.hpp>

namespace OPENTREP {

  // Forward declarations
  class ResultHolder;
  struct PlaceKey;


  // //////////////////// Type definitions /////////////////////
  /**
   * Pair of a Xapian document and its associated score board.
   */
  typedef std::pair<Xapian::Document, ScoreBoard> XapianDocumentPair_T;

  /**
   * (STL) List of Xapian documents and their associated score board.
   */
  typedef std::list<XapianDocumentPair_T> DocumentList_T;
  
  /**
   * (STL) Map of Xapian documents and their associated score board.
   */
  typedef std::map<Xapian::docid, XapianDocumentPair_T> DocumentMap_T;
  

  // //////////////////////// Main Class /////////////////////////
  /**
   * @brief Class wrapping a set of Xapian documents having matched a
   *        given query string.
   */
  class Result : public BomAbstract {
    friend class FacResultHolder;
    friend class FacResult;
  public:
    // ////////////// Getters /////////////
    /**
     * Get the query string.
     */
    const TravelQuery_T& getQueryString() const {
      return _queryString;
    }

    /**
     * Get the corrected query string.
     * When empty, it means that no correction was necessary.
     */
    const TravelQuery_T& getCorrectedTravelQuery() const {
     return _correctedQueryString;
    }

    /**
     * State whether there has been a full-text match.
     */
    bool hasFullTextMatched() const {
      return _hasFullTextMatched;
    }

    /**
     * Get the edit distance/error, with which the matching has been made.
     */
    const NbOfErrors_T& getEditDistance() const {
      return _editDistance;
    }
    
    /**
     * Get the maximal allowable edit distance/error, with which the
     * matching has been made.
     */
    const NbOfErrors_T& getAllowableEditDistance() const {
      return _allowableEditDistance;
    }
    
    /**
     * Get the list of documents.
     */
    const DocumentList_T& getDocumentList() const {
     return _documentList;
    }

    /**
     * Get the map of documents.
     */
    const DocumentMap_T& getDocumentMap() const {
     return _documentMap;
    }

    /**
     * Get the Xapian ID of the best matching document.
     */
    const Xapian::docid& getBestDocID() const {
      return _bestDocID;
    }

    /**
     * Get the combined weight, for all the rules (full-text, PageRank, etc)
     */
    const Percentage_T& getBestCombinedWeight() const {
      return _bestCombinedWeight;
    }

    /**
     * Get the best matching Xapian document.
     */
    const Xapian::Document& getBestXapianDocument() const;


  public:
    // ////////////// Setters /////////////
    /**
     * Set the query string.
     */
    void setQueryString (const TravelQuery_T& iQueryString) {
      _queryString = iQueryString;
    }

    /**
     * Set the corrected query string.
     */
    void setCorrectedQueryString (const TravelQuery_T& iCorrectedQueryString) {
      _correctedQueryString = iCorrectedQueryString;
    }
    
    /**
     * Set whether there has been a full-text match.
     */
    void setHasFullTextMatched (const bool iHasFullTextMatched) {
      _hasFullTextMatched = iHasFullTextMatched;
    }

    /**
     * Set the edit distance/error, with which the matching has been made.
     */
    void setEditDistance (const NbOfErrors_T& iEditDistance) {
      _editDistance = iEditDistance;
    }
    
    /**
     * Set the maxiaml allowable edit distance/error, with which the
     * matching has been made.
     */
    void setAllowableEditDistance (const NbOfErrors_T& iAllowableEditDistance) {
      _allowableEditDistance = iAllowableEditDistance;
    }
    
    /**
     * Set the latest score for the given type. If no score value has
     * already been stored for that type, create it.
    void setScore (const ScoreType& iScoreType, const Score_T& iScore) {
      _scoreBoard.setScore (iScoreType, iScore);
    }
     */

    /**
     * Add a Xapian document to the dedicated (STL) list and (STL) map.
     *
     * \note The score type is not specified, as it is corresponding,
     *       by construction, to the (Xapian-based) full-text matching.
     *       Indeed, when there is no (Xapian-based) full-text matching,
     *       by construction, no Xapian document has been found matching
     *       a given string.
     *
     * @param const Xapian::Document& The Xapian document to be added.
     * @param const Score_T& The matching percentage.
     */
    void addDocument (const Xapian::Document&, const Score_T&);

    /**
     * Set the Xapian ID of the best matching document.
     */
    void setBestDocID (const Xapian::docid& iDocID) {
      _bestDocID = iDocID;
    }

    /**
     * Set the best combined weight, for all the rules (full-text,
     * PageRank, etc)
     */
    void setBestCombinedWeight (const Percentage_T& iPercentage) {
      _bestCombinedWeight = iPercentage;
    }
    
    /**
     * Extract the best matching Xapian documents.
     *
     * @param Xapian::MSet& The Xapian matching set. It can be empty.
     * @param Result& The holder for the Xapian documents
     *        to be stored.
     */
    // //////////////////////////////////////////////////////////////////////
    void fillResult (const Xapian::MSet& iMatchingSet);


  public:
    // /////////// Business support methods /////////
    /**
     * For all the elements (strings) of the travel query (string set),
     * perform a Xapian-based full-text match.
     * That Xapian-based full-text match gives (potentially) a full set of
     * matches, some with the highest matching percentage and some with a
     * lower percentage.
     *
     * @param const Xapian::Database& The Xapian index/database.
     * @param TravelQuery_T& The query string.
     */
    std::string fullTextMatch (const Xapian::Database&, const TravelQuery_T&);

    /**
     * Extract the primary key from the data of the given Xapian document.
     *
     * The primary key is made of the first three words (IATA and ICAO
     * codes, as well as the Geonames ID) of the Xapian document
     * data/content.
     *
     * @param Xapian::Document& The Xapian document.
     * @return PlaceKey& The primary key of the place/POR (point of
     *         reference).
     */
    static PlaceKey getPrimaryKey (const Xapian::Document&);

    /**
     * Extract the PageRank from the data of the given Xapian document.
     *
     * The PageRank is the fourth word of the Xapian document data/content.
     *
     * @param Xapian::Document& The Xapian document.
     * @return Percentage_T& The PageRank of the place/POR (point of reference).
     */
    static Percentage_T getPageRank (const Xapian::Document&);

    /**
     * Calculate/set the PageRanks for all the matching documents
     */
    void calculatePageRanks();

    /**
     * Calculate/set the user input weights for all the matching documents
     */
    void calculateUserInputWeights();

    /**
     * Calculate/set the combined weights for all the matching documents.
     * Store the best matching one in the _bestCombinedWeight attribute.
     */
    void calculateCombinedWeights();

  private:
    /**
     * For all the elements (strings) of the travel query (string set),
     * perform a Xapian-based full-text match.
     * That Xapian-based full-text match gives (potentially) a full set of
     * matches, some with the highest matching percentage and some with a
     * lower percentage.
     *
     * @param const Xapian::Database& The Xapian index/database.
     * @param TravelQuery_T& The query string.
     * @param Xapian::MSet& The resulting matching set of Xapian documents
     */
    std::string fullTextMatch (const Xapian::Database&, const TravelQuery_T&,
                               Xapian::MSet&);


  public:
    // /////////// Display support methods /////////
    /**
     * Dump a Business Object into an output stream.
     *
     * @param ostream& the output stream.
     */
    void toStream (std::ostream& ioOut) const;

    /**
     * Read a Business Object from an input stream.
     *
     * @param istream& the input stream.
     */
    void fromStream (std::istream& ioIn);

    /**
     * Get the serialised version of the Business Object.
     */
    std::string toString() const;
    
    /**
     * Get a string describing the whole key (differentiating two objects
     * at any level).
     */
    std::string describeKey() const;

    /**
     * Get a string describing the short key (differentiating two objects
     * at the same level).
     */
    std::string describeShortKey() const;


  private:
    // ////////////// Constructors and Destructors /////////////
    /**
     * Main constructor.
     */
    Result (const TravelQuery_T&, const Xapian::Database&);

    /**
     * Default constructor.
     */
    Result();

    /**
     * Default copy constructor.
     */
    Result (const Result&);

    /**
     * Destructor.
     */
    ~Result();

    /**
     * Initialise (reset the list of documents).
     */
    void init();

    
  private:
    // /////////////// Attributes ////////////////
    /**
     * Parent ResultHolder.
     */
    ResultHolder* _resultHolder;

    /**
     * Xapian database.
     */
    const Xapian::Database& _database;

    /**
     * Query string having generated the set of documents.
     */
    TravelQuery_T _queryString;

    /**
     * That string reflects any spelling corrections, that Xapian may have
     * been suggested.
     */
    TravelQuery_T _correctedQueryString;

    /**
     * Whether or not there has been a full-text match.
     *
     * When there has been a full-text match, the set of documents contains
     * those matches. Otherwise, the set of documents is empty.
     */
    bool _hasFullTextMatched;

    /**
     * Edit distance/error, with which the matching has been made.
     */
    NbOfErrors_T _editDistance;
    
    /**
     * Maximum allowable edit distance/error, with which the matching
     * has been made.
     */
    NbOfErrors_T _allowableEditDistance;
    
    /**
     * Xapian ID of the best matching document.
     */
    Xapian::docid _bestDocID;

    /**
     * Best weight for all the rules (e.g., full-text matching,
     * PageRank, heuristic, popularity) for all the documents collected
     * so far.
     */
    Percentage_T _bestCombinedWeight;

    /**
     * (STL) List of Xapian documents and their associated score board.
     */
    DocumentList_T _documentList;
  
    /**
     * (STL) Map of Xapian documents and their associated score board.
     */
    DocumentMap_T _documentMap;
  };

}
#endif // __OPENTREP_BOM_RESULT_HPP
