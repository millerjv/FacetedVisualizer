/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// .NAME vtkSlicerFacetedVisualizerLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerFacetedVisualizerLogic_h
#define __vtkSlicerFacetedVisualizerLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRMLScene.h"
// STD includes
#include <cstdlib>

#include "vtkSlicerFacetedVisualizerModuleLogicExport.h"

#include <string>
#include <vector>
#include <map>
#include <utility>
//#include <std::multimap.h>

#ifndef WIN32 
#include <multimap.h>
#endif

#include <vtk_sqlite3.h>

#include <vtkMRMLModelHierarchyNode.h>

/// \ingroup Slicer_QtModules_FacetedVisualizer
class VTK_SLICER_FACETEDVISUALIZER_MODULE_LOGIC_EXPORT vtkSlicerFacetedVisualizerLogic :
  public vtkSlicerModuleLogic
{
public:
  
  static vtkSlicerFacetedVisualizerLogic *New();
  vtkTypeMacro(vtkSlicerFacetedVisualizerLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);


  bool ProcessQuery();

 //BTX
  void SynchronizeAtlasWithDB(std::vector< std::vector< std::string > >&matchingDBAtoms,
   		  std::vector< std::string > &unMatchedMRMLAtoms);

  void GetQueryResults( std::vector< std::vector < std::string > > &results,
     		std::vector< std::string> &queries);

  void SetCorrespondingDBTermforMRMLNode(std::string DBAtom, std::string mrmlNode)
  {
   	  mrmlDBTerms.insert(std::pair< std::string, std::string> (DBAtom, mrmlNode));
  }
//ETX

  //void AddQueryResultToCache(std::string &text);

  
  void SetDBFileName(std::string fname)
  {
	  dbFileName = fname;
          setValidDBFileName = true;
  }


  void SetQuery(std::string newquery)
  {
	  query = newquery;
  }

  std::string GetQuery()
  {
	  return query;
  }


protected:
  vtkSlicerFacetedVisualizerLogic();
  virtual ~vtkSlicerFacetedVisualizerLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);



private:

  // Helper methods
    void toLower(std::string origstr, std::string& newstr);

    void toUpper(std::string origstr, std::string& newstr);

    char* ProduceQuery(std::string& string, bool asObject,
    		             bool asSubject, std::string Predicate);

    void removeLeadingFollowingSpace(std::string& origstr);

//BTX

    int AddQueryResult(std::string &text, std::vector< std::string >& store);

    std::string GetDBSubject(std::string& query, vtk_sqlite3* ptrDB);

    ///////////////////////////////////////////////////////////////////////////////
    void SyncModelWithDB(vtkMRMLModelHierarchyNode *modelNode, vtk_sqlite3* ptrDB,
     		  std::vector< std::string > &possibleMatches);

    int ProcessSingleQuery(std::string& query, vtk_sqlite3* ptrDB,
    		std::vector< std::string > &queryResults,
    		std::vector< std::string > &displayTerms);


    int RecursiveProcessQuery(char* queryText, vtk_sqlite3* ptrDB,
  		  	  	  	  	  	bool queryAsSubject,
  		                    std::vector< std::string> &displayTerms);


    // Cache management -- currently commented out in the cxx file. HV
    int ManageQueryCache(std::vector< std::string >& queries,
  		  std::string query, std::vector< std::string> & queryResults);


 //ETX
private:

 //BTX
  std::string                             dbFileName;
  std::string                             query;

  std::vector < std::string >              recursionPredicates;

  std::vector< std::string >               addRecursionPredicates;

  std::vector< std::string >               ignorePredicates;

  std::vector< std::string >               commentPredicates;

   // do we cache results? How much?
  std::multimap< std::string, int > queryCacheMap;
  std::vector< std::string > queryResultCache;

  std::map< std::string, std::string> eqQueryMap;

  // how often is a specific query accessed. The least frequent query is the one that
  // is removed from the cache first
  std::map< std::string, int  >          queryResultAge;

  std::vector< std::string >             resultsForDisplay;

  std::multimap< std::string, std::string >             mrmlDBTerms;

  std::vector< std::string >             nonDBElements; // these are models that are added by the user to the scene
//ETX
  int                                  maxQueryHistory;

  int                                  cacheSize;

  bool                                 setValidDBFileName;
  // private methods
  vtkSlicerFacetedVisualizerLogic(const vtkSlicerFacetedVisualizerLogic&); // Not implemented
  void operator=(const vtkSlicerFacetedVisualizerLogic&);               // Not implemented


};

#endif

