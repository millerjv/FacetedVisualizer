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

#include <vcl_string.h>
#include <vcl_vector.h>
#include <vcl_map.h>
#include <vcl_utility.h>
//#include <std::multimap.h>
#include <multimap.h>
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
  void SynchronizeAtlasWithDB(vcl_vector< vcl_vector< vcl_string > >&matchingDBAtoms,
   		  vcl_vector< vcl_string > &unMatchedMRMLAtoms);

  void GetQueryResults( vcl_vector< vcl_vector < vcl_string > > &results,
     		vcl_vector< vcl_string> &queries);

  void SetCorrespondingDBTermforMRMLNode(vcl_string DBAtom, vcl_string mrmlNode)
  {
   	  mrmlDBTerms.insert(vcl_pair< vcl_string, vcl_string> (DBAtom, mrmlNode));
  }
//ETX

  //void AddQueryResultToCache(vcl_string &text);

  
  void SetDBFileName(vcl_string fname)
  {
	  dbFileName = fname;
  }


  void SetQuery(vcl_string newquery)
  {
	  query = newquery;
  }

  vcl_string GetQuery()
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
    void toLower(vcl_string origstr, vcl_string& newstr);

    void toUpper(vcl_string origstr, vcl_string& newstr);

    char* ProduceQuery(vcl_string& string, bool asObject,
    		             bool asSubject, vcl_string Predicate);

    void removeLeadingFollowingSpace(vcl_string& origstr);

//BTX

    int AddQueryResult(vcl_string &text, vcl_vector< vcl_string >& store);

    vcl_string GetDBSubject(vcl_string& query, vtk_sqlite3* ptrDB);

    ///////////////////////////////////////////////////////////////////////////////
    void SyncModelWithDB(vtkMRMLModelHierarchyNode *modelNode, vtk_sqlite3* ptrDB,
     		  vcl_vector< vcl_string > &possibleMatches);

    int ProcessSingleQuery(vcl_string& query, vtk_sqlite3* ptrDB,
    		vcl_vector< vcl_string > &queryResults,
    		vcl_vector< vcl_string > &displayTerms);


    int RecursiveProcessQuery(char* queryText, vtk_sqlite3* ptrDB,
  		  	  	  	  	  	bool queryAsSubject,
  		                    vcl_vector< vcl_string> &displayTerms);


    // Cache management -- currently commented out in the cxx file. HV
    int ManageQueryCache(vcl_vector< vcl_string >& queries,
  		  vcl_string query, vcl_vector< vcl_string> & queryResults);


 //ETX
private:

 //BTX
  vcl_string                             dbFileName;
  vcl_string                             query;

  vcl_vector < vcl_string >              recursionPredicates;

  vcl_vector< vcl_string >               addRecursionPredicates;

  vcl_vector< vcl_string >               ignorePredicates;

  vcl_vector< vcl_string >               commentPredicates;

   // do we cache results? How much?
  std::multimap< vcl_string, int > queryCacheMap;
  vcl_vector< vcl_string > queryResultCache;

  vcl_map< vcl_string, vcl_string> eqQueryMap;

  // how often is a specific query accessed. The least frequent query is the one that
  // is removed from the cache first
  vcl_map< vcl_string, int  >          queryResultAge;

  vcl_vector< vcl_string >             resultsForDisplay;

    std::multimap< vcl_string, vcl_string >             mrmlDBTerms;

  vcl_vector< vcl_string >             nonDBElements; // these are models that are added by the user to the scene
//ETX
  int                                  maxQueryHistory;

  int                                  cacheSize;

  // private methods
  vtkSlicerFacetedVisualizerLogic(const vtkSlicerFacetedVisualizerLogic&); // Not implemented
  void operator=(const vtkSlicerFacetedVisualizerLogic&);               // Not implemented


};

#endif

