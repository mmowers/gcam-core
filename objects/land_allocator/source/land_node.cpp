/*! 
* \file land_node.cpp
* \ingroup Objects
* \brief LandNode class source file.
* \author James Blackwood
* \date $Date$
* \version $Revision$
*/

#include "util/base/include/xml_helper.h"

#include "land_allocator/include/land_node.h"
#include "land_allocator/include/unmanaged_land_leaf.h"
#include "land_allocator/include/forest_land_leaf.h"
#include "containers/include/scenario.h"

using namespace std;
using namespace xercesc;

extern Scenario* scenario;

/*! \brief Constructor.
* \author James Blackwood
*/
LandNode::LandNode(){
    const Modeltime* modeltime = scenario->getModeltime();
    const int maxper = modeltime->getmaxper();
    landAllocation.resize( maxper );
}

//! Destructor
LandNode::~LandNode() {
}

size_t LandNode::getNumChildren() const {
    return children.size();
}

const ALandAllocatorItem* LandNode::getChildAt( const size_t aIndex ) const {
    /*! \pre aIndex is less than the size of the child vector. */
    assert( aIndex < children.size() );
    return children[ aIndex ];
}

ALandAllocatorItem* LandNode::getChildAt( const size_t aIndex ) {
    /*! \pre aIndex is less than the size of the child vector. */
    assert( aIndex < children.size() );
    return children[ aIndex ];
}

/*! \brief Parses any attributes specific to derived classes
* \author James Blackwood
* \param nodeName The name of the curr node. 
* \param curr pointer to the current node in the XML input tree
*/
bool LandNode::XMLDerivedClassParse( const string& nodeName, const DOMNode* curr ){
    const Modeltime* modeltime = scenario->getModeltime();

    if ( nodeName == LandNode::getXMLNameStatic() ) {
        LandNode* newNode = new LandNode;
        addChild( newNode );
        newNode->XMLParse( curr );
    }
    else if ( nodeName == LandLeaf::getXMLNameStatic() ) {
        LandLeaf* newLeaf = new LandLeaf;
        addChild( newLeaf );
        newLeaf->XMLParse( curr );
    }
    else if ( nodeName == ForestLandLeaf::getXMLNameStatic() ) {
        LandLeaf* newLeaf = new ForestLandLeaf;
        addChild( newLeaf );
        newLeaf->XMLParse( curr );
    }
    else if ( nodeName == UnmanagedLandLeaf::getXMLNameStatic() ) {
        LandLeaf* newLeaf = new UnmanagedLandLeaf;
        addChild( newLeaf );
        newLeaf->XMLParse( curr );
    }
    else if( nodeName == "sigma" ) {
        sigma = XMLHelper<double>::getValue( curr );
    }
    else if( nodeName == "landAllocation" ) {
        XMLHelper<double>::insertValueIntoVector( curr, landAllocation, modeltime ); 
    }
    else {
        return false;
    }
    return true;
}

/*! \brief Write XML values specific to derived objects
*
* \author Steve Smith
*/
void LandNode::toInputXMLDerived( ostream& out, Tabs* tabs ) const {
    
    XMLWriteElement( sigma, "sigma", out, tabs );
    
    // write out for children
    for ( unsigned int i = 0; i < children.size(); i++ ) {
        children[i]->toInputXML( out, tabs );
    }

}

/*! \brief Write XML values to debug stream for this object.
*
* \author Steve Smith
*/
void LandNode::toDebugXMLDerived( const int period, std::ostream& out, Tabs* tabs ) const {
    XMLWriteElement( sigma, "sigma", out, tabs );

    // write out for children
    for ( unsigned int i = 0; i < children.size(); i++ ) {
        children[i]->toDebugXML( period, out, tabs );
    }
}

/*! \brief Get the XML node name for output to XML.
*
* This public function accesses the private constant string, XML_NAME.
* This way the tag is always consistent for both read-in and output and can be easily changed.
* This function may be virtual to be overriden by derived class pointers.
* \author James Blackwood
* \return The constant XML_NAME.
*/
const string& LandNode::getXMLName() const {
    return getXMLNameStatic();
}

/*! \brief Get the XML node name in static form for comparison when parsing XML.
*
* This public function accesses the private constant string, XML_NAME.
* This way the tag is always consistent for both read-in and output and can be easily changed.
* The "==" operator that is used when parsing, required this second function to return static.
* \note A function cannot be static and virtual.
* \author James Blackwood
* \return The constant XML_NAME as a static.
*/
const std::string& LandNode::getXMLNameStatic() {
    const static string XML_NAME = "LandAllocatorNode";
    return XML_NAME;
}

/*! \brief Complete the Initialization in the LandAllocator.
* \author James Blackwood
*/
void LandNode::completeInit( const string& aRegionName,
                             const IInfo* aRegionInfo )
{
    for ( unsigned int i = 0; i < children.size(); i++ ) {
        children[ i ]->completeInit( aRegionName, aRegionInfo );
    }
}

void LandNode::addLandUsage( const string& aLandType,
                             const string& aProductName )
{
    // Find the parent land item which should have a leaf added.
    ALandAllocatorItem* parent = findItem ( aLandType );

    // Check that the parent exists.
    if( !parent ){
        ILogger& mainLog = ILogger::getLogger( "main_log" );
        mainLog.setLevel( ILogger::WARNING );
        mainLog << "Cannot add a land usage for " << aProductName << " as the land type "
                << aLandType << " does not exist." << endl;
    }
    else {
        // Add a new leaf for the land usage.
        LandLeaf* newLeaf = new LandLeaf;
        newLeaf->setName( aProductName );
        parent->addChild( newLeaf );
    }
}

/*! \brief Sets land allocation of unmanaged land nodes and leafs.
* \details Production nodes have their land allocation set by the supply sectors
*          that create them The land allocation in unmanaged land nodes need to
*          be set as the difference between the land allocated above and the
*          land used in the rest of the children at this level. Unmanaged land
*          leafs have an allocation read in, which acts as a relative share of
*          their land -- this needs to be adjusted to be consistant with the
*          land specified to be used in the production sectors root node is
*          represented by having a landAllocationAbove equal to zero.
* \warning this routine assumes that all leafs under an unmanaged land node are
*          not managed land.
* \todo Generalize this method so that unmanaged land leafs could be at any
*       level
* \param landAllocationIn Total land allocated to be allocated to this node
* \param period Period index
* \author Steve Smith
*/
void LandNode::setUnmanagedLandAllocation( const string& aRegionName,
                                           const double aLandAllocation,
                                           const int aPeriod )
{
    // If this node is not full of production leafs, then this is unmanaged land
    if ( !isProductionLeaf() && aLandAllocation > 0 ) {
        landAllocation[ aPeriod ] = aLandAllocation;
        if ( landAllocation[ aPeriod ] < 0 ) {
            ILogger& mainLog = ILogger::getLogger( "main_log" );
            mainLog.setLevel( ILogger::DEBUG );
            mainLog << "Land allocation above is less than land allocated." << endl;
            landAllocation[ aPeriod ] = 0;
        }

        double totalLandAllocated = getLandAllocation( name, aPeriod );
        if ( totalLandAllocated > 0 ) {
            double landAllocationScaleFactor = landAllocation[ aPeriod ] / totalLandAllocated;
            for ( unsigned int i = 0; i < children.size() ; i++ ) {
                double childLandAllocation = children[ i ]->getLandAllocation( children[ i ]->getName(), aPeriod );
                double newAllocation = childLandAllocation * landAllocationScaleFactor;
                children[ i ]->setUnmanagedLandAllocation( aRegionName, newAllocation, aPeriod );
            }
        }
        calcLandShares( aRegionName, 0, 0, aPeriod );
    }
}

/*! \brief Sets the initial shares and land allocation.
* root node is represented by having a landAllocationAbove equal to zero.
* \warning Unmanged land allocations are not set properly. 
* \todo figure out a way to set the unmanaged land allocation leaves. (Unmanaged Class)
* \author James Blackwood
*/
void LandNode::setInitShares( double landAllocationAbove, int period ) {
    //Summing the LandAllocations of the children
    landAllocation[ period ] = getTotalLandAllocation( name, period ); 

    //Calculating the shares
    for ( unsigned int i = 0; i < children.size(); i++ ) {
        children[ i ]->setInitShares( landAllocation[ period ], period );
    }

    share[ period ] = landAllocation[ period ] / landAllocationAbove;
}

/*! \brief Sets the intrinsicYieldMode recursively.
* root node is represented by having an intrinsicYieldModeAbove equal to one.
* \author James Blackwood
*/
void LandNode::setIntrinsicYieldMode( double intrinsicRateAbove, double sigmaAbove, int period ) {
    double intrinsicRateToBePassed = intrinsicRateAbove * pow( share[period], sigmaAbove );
    if( intrinsicRateToBePassed > 0 ){
        for ( unsigned int i = 0; i < children.size(); i++ ) {
            children[ i ]->setIntrinsicYieldMode( intrinsicRateToBePassed, sigma, period );
        }
    }
}

/*! \brief Finds the location to set the intrinsicRate and sets it using the leaf's version of this method.
* If the location is not found it creates it as a leaf from the landType which should be a node.
* \author Josh Lurz, James Blackwood
*/
void LandNode::setIntrinsicRate( const string& aRegionName,
                                 const string& aLandType,
                                 const string& aProductName,
                                 const double aIntrinsicRate, 
                                 const int aPeriod )
{
    ALandAllocatorItem* curr = findItem ( aProductName );

    // Set the rental rate.
    if( curr ){
        curr->setIntrinsicRate( aRegionName, aLandType, aProductName, aIntrinsicRate, aPeriod );
    }
}

/*! \brief Finds the location to set the calibrated land allocation values and
*          sets them using the leaf's version of this method. If the location is
*          not found it creates it as a leaf from the landType which should be a
*          node.
* \author James Blackwood
*/
void LandNode::setCalLandAllocation( const string& aLandType,
                                     const string& aProductName,
                                     const double aCalLandUsed,
                                     const int aHarvestPeriod, 
                                     const int aCurrentPeriod )
{
    ALandAllocatorItem* curr = findItem ( aProductName );

    // Set the land allocation.
    if( curr ){
        curr->setCalLandAllocation( aLandType, aProductName, aCalLandUsed,
                                    aHarvestPeriod, aCurrentPeriod );
    }
}

/*! \brief Finds the location to set the calibrated land allocation and observed
*          yield values and sets them using the leaf's version of this method.
*          If the location is not found it creates it as a leaf from the
*          landType which should be a node.
* \author James Blackwood
*/
void LandNode::setCalObservedYield( const string& aLandType,
                                    const string& aProductName,
                                    const double aCalObservedYield,
                                    const int aPeriod )
{
    ALandAllocatorItem* curr = findItem ( aProductName );
    
    if( curr ){
        curr->setCalObservedYield( aLandType, aProductName, aCalObservedYield, aPeriod );
    }
}

/*! \brief Returns the IntrinsicRate of this land type at the specified period,
*          if available.
* \details The intrinsic rate is divided by the share, and later divided by
*          yield, and then subtracted from price to get the variable cost. VC =
*          P - (intrinsicRate /(share * yield))
* \author James Blackwood
*/
double LandNode::getCalAveObservedRateInternal( const string& aLandType,
                                                const int aPeriod,
                                                const double aSigma ) const
{
    // Check if this node is the requested node.
    if( name == aLandType ){
        return intrinsicRate[ aPeriod ] / pow( share[ aPeriod ], aSigma );
    }

    // Otherwise perform a search for the land type.
    const ALandAllocatorItem* curr = findItem( aLandType );
    
    if( curr ){
        return curr->getCalAveObservedRateInternal( aLandType, aPeriod, aSigma );
    }
    return 0;
}

/*! \brief Finds the location to apply the agruculture production change
* and sets them using the leaf's version of this method. If the location is not
* found it creates it as a leaf from the landType which should be a node.
* \author James Blackwood
*/
void LandNode::applyAgProdChange( const string& aLandType,
                                  const string& aProductName,
                                  const double aAgProdChange,
                                  const int aPeriod )
{
    ALandAllocatorItem* curr = findItem( aProductName );
    
    if( curr ){
        curr->applyAgProdChange( aLandType, aProductName, aAgProdChange, aPeriod );
    }
}

/*! \brief This method will calculate the share value for each leaf and node,
*          and then normalize it.
* \details This function will be called from the supply side of the sectors and
*          will be passed a default dummy sigmaAbove. The first loop cycles
*          through all the children. If a child is a leaf then it will call the
*          calcLandShares method in LandAllocatorLeaf, where share is
*          calculated. If a child is a node there will be a recursive call to
*          this method. The second loop uses the sum of all the shares of the
*          children vector and normalizes and overwrites share. Finally, share
*          is calculated for this node using the calculated intrinsicRate and
*          the sigma from one level up.
* \param aSigmaAbove the sigma value from the node above this level.
* \author James Blackwood
* \todo need a better way to check if "UnmanagedLand" to not overwrite
*       intrinsicRate that was read in through input
* \todo May need to add a method to deal with case if total allocation is
*       greater than initial allocation 
* \todo this will not work if unmanaged land nodes are nested
*/
void LandNode::calcLandShares( const string& aRegionName,
                               const double aSigmaAbove,
                               const double aTotalLandAllocated,
                               const int aPeriod )
{
    // First adjust value of unmanaged land nodes
    setUnmanagedLandValues( aRegionName, aPeriod );

    // Calculate the temporary unnormalized shares and sum them
    double unnormalizedSum = 0;
    // double excessShares = 0; (ignore this for now)
    double totalBaseLandAllocation = getBaseLandAllocation( aPeriod );
    for ( unsigned int i = 0; i < children.size(); i++ ) {
        // If this is managed land, or if unmanaged land and no base land
        // allocation is specified then use standard method.
        if ( isProductionLeaf() || totalBaseLandAllocation == 0 ) {
            children[ i ]->calcLandShares( aRegionName, sigma, 0, aPeriod );
        }
        else {
            // If this is unmanaged land then use initial land allocation to
            // weight land use.
            children[ i ]->calcLandShares( aRegionName, sigma, totalBaseLandAllocation, aPeriod );
        }
        // Get the temporary unnormalized share.
        unnormalizedSum += children[ i ]->getShare( aPeriod );                   
    }

    // Normalizing the temporary unnormalized shares
    for ( unsigned int i = 0; i < children.size(); i++ ) {
        children[ i ]->normalizeLandAllocation( unnormalizedSum, aPeriod );
    }

    // The unnormalizedSum is actually the 1/sigma weighted intrinsic rates of
    // the children Therefore, the equation below gives the intrinsic rate of
    // this node
    intrinsicRate[ aPeriod ] = pow( unnormalizedSum, sigma );
    
    // This is a temporary unnormalized share.  
    share[ aPeriod ] = pow( intrinsicRate[ aPeriod ], 1 / aSigmaAbove ); 
}

/*! \brief Adjust land values for unmanaged land nodes as necessary
* \param aRegionName Region name.
* \param aPeriod Model period
* \author Steve Smith
*/
void LandNode::setUnmanagedLandValues( const string& aRegionName, const int aPeriod ) {
    for ( unsigned int i = 0; i < children.size() ; i++ ) {
        children[ i ]->setUnmanagedLandValues( aRegionName, aPeriod );
    }
}

/*! \brief Recursively calculates the landAllocation at each leaf and node using
*          the shares. landAllocationAbove is passed the value of 0 at the root
*          when this method is called, so the value in landAllocation at the
*          root will not be changed and be passed down recursively.
* \author Steve Smith, James Blackwood
*/
void LandNode::calcLandAllocation( double landAllocationAbove, int period ) {
    landAllocation[ period ] = landAllocationAbove * share[ period ];
    for ( unsigned int i = 0; i < children.size(); i++ ) {
        children[ i ]->calcLandAllocation( landAllocation[ period ], period );
    }
}

/*! \brief Finds the location to calculate the yield using the leaf's version of this method.
* If the location is not found it creates it as a leaf from the landType which should be a node.
* \author James Blackwood
*/
void LandNode::calcYieldInternal( const string& aLandType,
                                  const string& aProductName,
                                  const double aProfitRate,
                                  const double aAvgIntrinsicRate,
                                  const int aPeriod )
{
    ALandAllocatorItem* curr = findItem( aProductName );
    
    if( curr ){
        curr->calcYieldInternal( aLandType, aProductName, aProfitRate, aAvgIntrinsicRate, aPeriod );
    }
}

/*! \brief Finds the location to get the yield using the leaf's version of this method.
* If the location is not found it creates it as a leaf from the landType which should be a node.
* \author James Blackwood
*/
double LandNode::getYield( const string& landType, const string& productName, const int period ) const {
    const ALandAllocatorItem* curr = findItem( productName );

    if( curr ){
        return curr->getYield( landType, productName, period );
    }
    return 0;
}

/*! \brief Adds a child to the children vector by pushing it on the back.
* The child can be a leaf or a node.
* \todo This doesn't check for the existance of a node.
* \author Josh Lurz, James Blackwood
*/
void LandNode::addChild( ALandAllocatorItem* child ) {
    /*! \pre The child exists. */
    assert( child );

    // Check if the child already exists.
    ALandAllocatorItem* existingItem = findItem( child->getName() );
    if( existingItem ){
        // TODO: Uncomment this warning. Currently all technologies with the
        // same type but different periods will try to add the land type and
        // cause this warning.
        /*
        ILogger& mainLog = ILogger::getLogger( "main_log" );
        mainLog.setLevel( ILogger::WARNING );
        mainLog << "Land type " << name << " already has a child named " << child->getName() << "." << endl;
        */
        delete child;
        return;
    }
    children.push_back( child );
}

/*! \brief Recursively sums the landAllocation of all the nodes and leafs below
*          this landType.
* \author James Blackwood
* \return the landAllocation of this landType
* \todo Does this continue searching after finding the land type?
*/
double LandNode::getLandAllocation( const string& aProductName,
                                    const int aPeriod ) const 
{
    double sum = 0;
    for ( unsigned int i = 0; i < children.size(); i++ ) {
        if ( name == aProductName ) {
            sum += getLandAllocation( children[ i ]->getName(), aPeriod );
        }
        else {
            sum += children[ i ]->getLandAllocation( aProductName, aPeriod );
        }
    }
    return sum;
}

/*! \brief Returns all land allocated for this land type.
* \details This function is needed so that separate function can be called for
*          land classes with vintaging when total land allocated is necessary
* \author Steve Smith
* \return the land Allocated to this landType
*/
double LandNode::getTotalLandAllocation( const string& productName, int period ) {
    double sum = 0;
    for ( unsigned int i = 0; i < children.size(); i++ ) {
        if ( name == productName ) {
            sum += getTotalLandAllocation( children[ i ]->getName(), period );
        }
        else {
            sum += children[ i ]->getTotalLandAllocation( productName, period );
        }
    }
    return sum;
}

/*! \brief Recursively sums the baseLandAllocation of all the nodes and leafs
*          below this landType.
*
* Only sums land in unmanaged land nodes, since these are the only ones that
* have a base land allocation
*
* \author Steve Smith
* \return the baseLandAllocation of this landType
*/
double LandNode::getBaseLandAllocation ( int period ) {
    double sum = 0;
    for ( unsigned int i = 0; i < children.size(); i++ ) {
        if ( !children[i]->isProductionLeaf() ) {
            sum += children[ i ]->getBaseLandAllocation( period );
        }
    }
    return sum;
}

/*! \brief Returns Whether all leaves under this node are production leaves.
* \details Checks all nodes and leaf below this node, if any are not a
*          production leaf this returns false.
* \return Whether all children leaves are production leaves.
* \author Steve Smith
*/
bool LandNode::isProductionLeaf() const {
    for ( unsigned int i = 0; i < children.size(); i++ ) {
        if ( !children[ i ]->isProductionLeaf() ) {
            return false;
        }
    }
    return true;
}

/*! \brief Write output to csv output file. 
*
*
* \author Steve Smith
*/
void LandNode::csvOutput( const string& aRegionName ) const {
    ALandAllocatorItem::csvOutput( aRegionName );
    //write output for children
    for ( unsigned int i = 0; i < children.size(); i++ ) {
        children[ i ]->csvOutput( aRegionName );
    }
}

void LandNode::dbOutput( const string& aRegionName ) const {
    ALandAllocatorItem::dbOutput( aRegionName );
    //write output for children
    for ( unsigned int i = 0; i < children.size(); i++ ) {
        children[ i ]->dbOutput( aRegionName );
    }
}

void LandNode::calcEmission( const string& aRegionName,
                             const GDP* aGDP, 
                             const int aPeriod )
{
    for ( unsigned int i = 0; i < children.size(); i++ ) {
        children[ i ]->calcEmission( aRegionName, aGDP, aPeriod );
    }
}

void LandNode::updateSummary( Summary& aSummary, const int aPeriod ) {
    for ( unsigned int i = 0; i < children.size(); i++ ) { 
        children[ i ]->updateSummary( aSummary, aPeriod );
    }
}
