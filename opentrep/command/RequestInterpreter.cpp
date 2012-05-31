// //////////////////////////////////////////////////////////////////////
// Import section
// //////////////////////////////////////////////////////////////////////
// STL
#include <cassert>
#include <sstream>
#include <string>
#include <vector>
#include <exception>
// OpenTrep
#include <opentrep/bom/Filter.hpp>
#include <opentrep/bom/WordHolder.hpp>
#include <opentrep/bom/Place.hpp>
#include <opentrep/bom/ResultCombination.hpp>
#include <opentrep/bom/ResultHolder.hpp>
#include <opentrep/bom/Result.hpp>
#include <opentrep/bom/PlaceHolder.hpp>
#include <opentrep/bom/StringPartition.hpp>
#include <opentrep/factory/FacPlaceHolder.hpp>
#include <opentrep/factory/FacPlace.hpp>
#include <opentrep/factory/FacResultCombination.hpp>
#include <opentrep/factory/FacResultHolder.hpp>
#include <opentrep/factory/FacResult.hpp>
#include <opentrep/command/DBManager.hpp>
#include <opentrep/command/RequestInterpreter.hpp>
#include <opentrep/service/Logger.hpp>
// Xapian
#include <xapian.h>

namespace OPENTREP {

  /**
   * Helper function to add the given string to the list of unknown words,
   * only when that given string is made of a single word. Otherwise, the
   * given string is simply ignored.
   *
   * @param const TravelQuery_T& The given string
   * @param WordList_T& ioWordList The list to which the given single-word
   *        string should be added (if needed).
   * @param WordSet_T& A (STL) set of words, allowing to keep track of
   *        the already added words.
   */
  // //////////////////////////////////////////////////////////////////////
  void addUnmatchedWord (const TravelQuery_T& iQueryString,
                         WordList_T& ioWordList, WordSet_T& ioWordSet) {
    // Token-ise the given string
    WordList_T lQueryStringWordList;
    WordHolder::tokeniseStringIntoWordList (iQueryString,
                                            lQueryStringWordList);
    if (lQueryStringWordList.size() == 1) {
      // Add the unmatched/unknown word, only when that latter has not
      // already been stored, and when it is not black-listed.
      const bool shouldBeKept = Filter::shouldKeep ("", iQueryString);
              
      WordSet_T::const_iterator itWord = ioWordSet.find (iQueryString);
      if (shouldBeKept == true && itWord == ioWordSet.end()) {
        ioWordSet.insert (iQueryString);
        ioWordList.push_back (iQueryString);
      }
    }
  }

  /**
   * Helper function to retrieve, from the database, the details of a
   * given point of reference (POR). The corresponding Place BOM
   * object is then filled with those details.
   */
  // //////////////////////////////////////////////////////////////////////
  bool retrieveAndFillPlace (const std::string& iOriginalKeywords,
                             const std::string& iCorrectedKeywords,
                             const Xapian::Document& iDocument,
                             const Xapian::percent& iDocPercentage,
                             soci::session& ioSociSession, Place& ioPlace) {
    bool hasRetrievedPlace = false;

    // Set the original and corrected/suggested keywords
    ioPlace.setOriginalKeywords (iOriginalKeywords);
    ioPlace.setCorrectedKeywords (iCorrectedKeywords);
    
    // Set the matching percentage
    ioPlace.setPercentage (iDocPercentage);
    
    // Retrieve the parameters of the best matching document
    const PlaceKey& lKey = Result::getPrimaryKey (iDocument);

    // DEBUG
    const Xapian::docid& lDocID = iDocument.get_docid();
    const std::string& lDocData = iDocument.get_data();
    OPENTREP_LOG_DEBUG ("Place key: " << lKey << " - Xapian ID " << lDocID
                        << ", " << iDocPercentage << "% [" << lDocData << "]");

    // Fill the Place object with the row retrieved from the
    // (MySQL) database and corresponding to the given place code
    // (e.g., 'sfo' for the San Francisco Intl airport).
    hasRetrievedPlace = DBManager::retrievePlace (ioSociSession, lKey, ioPlace);

    if (hasRetrievedPlace == false) {
      /**
       * The Xapian database/index should contain only places
       * available within the SQL database, as the first is built from
       * the latter.  If that happens, it means that the user gave a
       * wrong Xapian database.
       */
      std::ostringstream errorStr;
      errorStr << "There is no document corresponding to " << lKey
               << " (Xapian document ID" << lDocID
               << " [" << lDocData << "]) in the SQL database. "
               << "It usually means that the Xapian index/database "
               << "is not synchronised with the SQL database. "
               << "[Hint] Rebuild the Xapian index/database "
               << "from the SQL database.";
      OPENTREP_LOG_ERROR (errorStr.str());
      throw XapianTravelDatabaseNotInSyncWithSQLDatabaseException (errorStr.str());
    }
    
    return hasRetrievedPlace;
  }
  
  /**
   * Helper function to retrieve, from the database, the details of a
   * given list of points of reference (POR). The corresponding Place
   * BOM objects are then filled according to the retrieved details.
   */
  // //////////////////////////////////////////////////////////////////////
  bool retrieveAndFillPlace (const Result& iResult,
                             soci::session& ioSociSession, Place& ioPlace) {
    // Note that Result::getTravelQuery() returns a TravelQuery_T,
    // which is actually a std::string
    const std::string& lOriginalKeywords = iResult.getQueryString();
    const std::string& lCorrectedKeywords = iResult.getCorrectedTravelQuery();
    
    // Delegate
    const Xapian::Document& lXapianDocument = iResult.getBestXapianDocument();
    const Xapian::percent& lWeight = iResult.getBestCombinedWeight();
    return retrieveAndFillPlace (lOriginalKeywords, lCorrectedKeywords,
                                 lXapianDocument, lWeight,
                                 ioSociSession, ioPlace);
  }
  
  // //////////////////////////////////////////////////////////////////////
  void createPlaces (const ResultCombination& iResultCombination,
                     soci::session& ioSociSession, PlaceHolder& ioPlaceHolder) {
    
    // Retrieve the best matching ResultHolder object.
    const ResultHolder& lResultHolder =
      iResultCombination.getBestMatchingResultHolder();    

    // Browse the list of result objects
    const ResultList_T& lResultList = lResultHolder.getResultList();
    for (ResultList_T::const_iterator itResult = lResultList.begin();
         itResult != lResultList.end(); ++itResult) {
      // Retrieve the result object
      const Result* lResult_ptr = *itResult;
      assert (lResult_ptr != NULL);

      /**
       * When there has been no full-text match, the Result object exists,
       * but, by construction, it does not correspond to any Xapian document.
       */
      const bool hasFullTextMatched = lResult_ptr->hasFullTextMatched();
      if (hasFullTextMatched == false) {
        continue;
      }
      assert (hasFullTextMatched == true);

      // Instanciate an empty place object, which will be filled from the
      // rows retrieved from the database.
      Place& lPlace = FacPlace::instance().create();
      
      // Retrieve, in the MySQL database, the place corresponding to
      // the place code located as the first word of the Xapian
      // document data.
      bool hasRetrievedPlace = retrieveAndFillPlace (*lResult_ptr,
                                                     ioSociSession, lPlace);

      // Retrieve the effective (Levenshtein) edit distance/error, as
      // well as the allowable edit distance/error, and store them in
      // the Place object.
      const NbOfErrors_T& lEditDistance = lResult_ptr->getEditDistance();
      const NbOfErrors_T& lAllowableEditDistance =
        lResult_ptr->getAllowableEditDistance();
      lPlace.setEditDistance (lEditDistance);
      lPlace.setAllowableEditDistance (lAllowableEditDistance);

      // If there was no place corresponding to the place code with
      // the SQL database, an exception is thrown. Hence, here, by
      // construction, the place has been retrieved from the SQL
      // database.
      assert (hasRetrievedPlace == true);

      // Insert the Place object within the PlaceHolder object
      FacPlaceHolder::initLinkWithPlace (ioPlaceHolder, lPlace);
      
      // DEBUG
      OPENTREP_LOG_DEBUG ("Retrieved Document: " << lPlace.toString());
    }
  }
  
  /**
   * For all the elements (StringSet) of the string partitions, derived
   * from the given travel query, perform a Xapian-based full-text match.
   * Each Xapian-based full-text match gives (potentially) a full set of
   * matches, some with the highest matching percentage and some with a
   * lower percentage.
   *
   * @param TravelQuery_T& The query string.
   * @param const Xapian::Database& The Xapian index/database.
   * @param ResultCombination& List of ResultHolder objects.
   * @param WordList_T& List of non-matched words of the query string.
   */
  // //////////////////////////////////////////////////////////////////////
  void searchString (const TravelQuery_T& iTravelQuery,
                     const Xapian::Database& iDatabase,
                     ResultCombination& ioResultCombination,
                     WordList_T& ioWordList) {

    // Catch any thrown Xapian::Error exceptions
    try {
      
      // Set of unknown words (just to eliminate the duplicates)
      WordSet_T lWordSet;

      // First, calculate all the partitions of the initial travel query
      StringPartition lStringPartition (iTravelQuery);

      // DEBUG
      OPENTREP_LOG_DEBUG ("+++++++++++++++++++++");
      OPENTREP_LOG_DEBUG ("Travel query: `" << iTravelQuery << "'");
      OPENTREP_LOG_DEBUG ("Partitions: " << lStringPartition);

      // Browse the partitions
      for (StringPartition::StringPartition_T::const_iterator itSet =
             lStringPartition._partition.begin();
           itSet != lStringPartition._partition.end(); ++itSet) {
        const StringSet& lStringSet = *itSet;

        // DEBUG
        OPENTREP_LOG_DEBUG ("  ==========");
        OPENTREP_LOG_DEBUG ("  String set: " << lStringSet);

        // Create a ResultHolder object.
        ResultHolder& lResultHolder =
          FacResultHolder::instance().create (lStringSet.describe(), iDatabase);

        // Add the ResultHolder object to the dedicated list.
        FacResultCombination::initLinkWithResultHolder (ioResultCombination,
                                                        lResultHolder);

        // Browse through all the word combinations of the partition
        for (StringSet::StringSet_T::const_iterator itString =
               lStringSet._set.begin();
             itString != lStringSet._set.end(); ++itString) {
          //
          const std::string lQueryString (*itString);

          // DEBUG
          OPENTREP_LOG_DEBUG ("    --------");
          OPENTREP_LOG_DEBUG ("    Query string: " << lQueryString);

          // Create an empty Result object
          Result& lResult = FacResult::instance().create (lQueryString,
                                                          iDatabase);
      
          // Add the Result object to the dedicated list.
          FacResultHolder::initLinkWithResult (lResultHolder, lResult);

          // Perform the Xapian-based full-text match: the set of
          // matching documents is filled.
          const std::string& lMatchedString =
            lResult.fullTextMatch (iDatabase, lQueryString);

          // When a single-word string is unmatched/unknown by/from Xapian,
          // add it to the dedicated list (i.e., ioWordList).
          if (lMatchedString.empty() == true) {
            OPENTREP::addUnmatchedWord (lQueryString, ioWordList, lWordSet);
          }
        }

        // DEBUG
        OPENTREP_LOG_DEBUG (std::endl
                            << "========================================="
                            << std::endl << "Result holder: "
                            << lResultHolder.toString()
                            << "========================================="
                            << std::endl << std::endl);
      }

      // DEBUG
      OPENTREP_LOG_DEBUG ("*********************");

    } catch (const Xapian::Error& error) {
      OPENTREP_LOG_ERROR ("Exception: "  << error.get_msg());
      throw XapianException (error.get_msg());
    }
  }

  /**
   * Select the best matching string partition, based on the results of
   * several rules, that are all materialised by weighting percentages:
   * <ul>
   *   <li>Xapian-based full-text match. Score type: XAPIAN_PCT</li>
   *   <li>Schedule-derived PageRank for the point of reference (POR).
   *       Score type: PAGE_RANK</li>
   *   <li>User input. Score type: USER_INPUT</li>
   * </ul>
   * \see ScoreType.hpp for the various types of score.
   *
   * The score for the combination (score type: COMBINATION) is simply the
   * product of all the scores/weighting percentages.
   *
   * @param ResultCombination& List of ResultHolder objects.
   */
  // //////////////////////////////////////////////////////////////////////
  void chooseBestMatchingResultHolder (ResultCombination& ioResultCombination) {

    // Calculate the weights for the full-text matches
    const bool doesBestMatchingResultHolderExist =
      ioResultCombination.chooseBestMatchingResultHolder();

    if (doesBestMatchingResultHolderExist == true) {
      const ResultHolder& lBestMatchingResultHolder =
        ioResultCombination.getBestMatchingResultHolder();

      // DEBUG
      OPENTREP_LOG_DEBUG ("Best matching ResultHolder: "
                          << lBestMatchingResultHolder);

    } else {
      // DEBUG
      OPENTREP_LOG_DEBUG ("No best matching ResultHolder object.");
    }
  }

  // //////////////////////////////////////////////////////////////////////
  NbOfMatches_T RequestInterpreter::
  interpretTravelRequest (soci::session& ioSociSession,
                          const TravelDatabaseName_T& iTravelDatabaseName,
                          const TravelQuery_T& iTravelQuery,
                          LocationList_T& ioLocationList,
                          WordList_T& ioWordList) {
    NbOfMatches_T oNbOfMatches = 0;

    // Sanity checks
    assert (iTravelDatabaseName.empty() == false);
    assert (iTravelQuery.empty() == false);

    // Create a PlaceHolder object, to collect the matching Place objects
    PlaceHolder& lPlaceHolder = FacPlaceHolder::instance().create();

    // Make the database
    Xapian::Database lXapianDatabase (iTravelDatabaseName);

    // DEBUG
    OPENTREP_LOG_DEBUG (std::endl
                        << "=========================================");
      
    /**
     * 0. Initialisation
     *
     * Create a ResultCombination BOM instance.
     */
    ResultCombination& lResultCombination =
      FacResultCombination::instance().create (iTravelQuery);

    /**
     * 1.1. Perform all the full-text matches, and fill accordingly the
     *      list of Result instances.
     */
    OPENTREP::searchString (iTravelQuery, lXapianDatabase, lResultCombination,
                            ioWordList);

    /**
     * 1.2. Calculate/set the PageRanks for all the matching documents
     */
    lResultCombination.calculatePageRanks();

    /**
     * 1.3. Calculate/set the user input weights for all the matching documents
     */
    lResultCombination.calculateUserInputWeights();

    /**
     * 1.4. Calculate/set the combined weights for all the matching documents
     */
    lResultCombination.calculateCombinedWeights();

    /**
     * 2. Calculate the best matching scores / weighting percentages.
     */
    OPENTREP::chooseBestMatchingResultHolder (lResultCombination);

    /**
     * 3. Create the list of Place objects, for each of which a
     *    look-up is made in the SQL database (e.g., MySQL or Oracle)
     *    to retrieve complementary data.
     */
    createPlaces (lResultCombination, ioSociSession, lPlaceHolder);
      
    // DEBUG
    OPENTREP_LOG_DEBUG (std::endl
                        << "========================================="
                        << std::endl << "Summary:" << std::endl
                        << lPlaceHolder.toShortString() << std::endl
                        << "========================================="
                        << std::endl);

    /**
     * 4. Create the list of Location structures, which are light copies
     *    of the Place objects. Those (Location) structures are passed
     *    back to the caller of the service.
     */
    lPlaceHolder.createLocations (ioLocationList);
    oNbOfMatches = ioLocationList.size();
    
    return oNbOfMatches;
  }
  
}
