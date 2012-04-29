#ifndef __OPENTREP_BOM_STRINGSET_HPP
#define __OPENTREP_BOM_STRINGSET_HPP

// //////////////////////////////////////////////////////////////////////
// Import section
// //////////////////////////////////////////////////////////////////////
// STL
#include <string>
#include <list>
// OpenTREP
#include <opentrep/OPENTREP_Types.hpp>
#include <opentrep/bom/BomAbstract.hpp>

namespace OPENTREP {

  /**
   * Class holding a set of strings, e.g., {"rio", "de", "janeiro"}
   */
  struct StringSet : public BomAbstract {
  public:
    // //////////////// Type definitions //////////////////
    /**
     * List of strings.
     */
    typedef std::list<std::string> StringSet_T;

  public:
    /**
     * Add an item (string) into the list.
     */
    void push_back (const std::string&);
  
    /**
     * Add all the items (string) into the list.
     */
    void push_back (const StringSet&);
  
    /**
     * Return the size of the list.
     */
    size_t size() const;
  
    /**
     * Empty the list.
     */
    void clear();
    
  public:
    // /////////// Display support methods /////////
    /**
     * Dump the structure into an output stream.
     *
     * @param ostream& the output stream.
     */
    void toStream (std::ostream&) const;

    /**
     * Read a structure from an input stream.
     *
     * @param istream& the input stream.
     */
    void fromStream (std::istream&);

    /**
     * Get the serialised version of the structure.
     */
    std::string toString() const;
    
    /**
     * Get a shorter serialised version of the structure.
     */
    std::string toShortString() const;
    
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


  public:
    // //////////////// Constructors and Destructors /////////////
    /**
     * Default constructor.
     */
    StringSet();

    /**
     * Default copy constructor.
     */
    StringSet (const StringSet&);

    /**
     * Main constructor.
     */
    StringSet (const std::string&);

    /**
     * Default destructor.
     */
    ~StringSet();

  
  public:
    // //////////////// Attributes ///////////////
    /**
     * String set, i.e., a list of strings
     */
    StringSet_T _set;
  };

}
#endif // __OPENTREP_BOM_STRINGSET_HPP