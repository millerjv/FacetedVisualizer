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

// FacetedVisualizer includes
#include "vtkSlicerFacetedVisualizerLogic.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkCollection.h"

// VTK includes
#include <vtkNew.h>

// STD includes
#include <cassert>
#include <string>
#include <algorithm>
#include <utility>
#include <string>
#include <cctype>


#include "vtk_sqlite3.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerFacetedVisualizerLogic);

//----------------------------------------------------------------------------
vtkSlicerFacetedVisualizerLogic::vtkSlicerFacetedVisualizerLogic()
{

  setValidDBFileName = false;
	// filter predicates for continuing recursive queries to DB
	recursionPredicates.push_back("regional_part");
	recursionPredicates.push_back("consitutional_part");
	recursionPredicates.push_back("systemic_part");
	recursionPredicates.push_back("member");
	recursionPredicates.push_back("subClass");

	addRecursionPredicates.push_back("regional_part_of");
	addRecursionPredicates.push_back("constitutional_part_of");
	addRecursionPredicates.push_back("systemic_part_of");
	addRecursionPredicates.push_back("memberOf");
	addRecursionPredicates.push_back("subClassOf");

	// filter predicates to ignore from adding to results
	ignorePredicates.push_back("label");
	ignorePredicates.push_back("fmaid");
	// these are required only for a specific DB that Harini had to modify to work with the
	// earlier implementation of Faceted Search Atlas
	ignorePredicates.push_back("assocmrmlnode");
	ignorePredicates.push_back("assocematlaslabel");
	ignorePredicates.push_back("hasematlasname");

	commentPredicates.push_back("comment");
	commentPredicates.push_back("definition");


	maxQueryHistory = 10; // lets use only 3 queries in the cache right now

	cacheSize = 3000;

}

//----------------------------------------------------------------------------
vtkSlicerFacetedVisualizerLogic::~vtkSlicerFacetedVisualizerLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerFacetedVisualizerLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerFacetedVisualizerLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerFacetedVisualizerLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerFacetedVisualizerLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerFacetedVisualizerLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerFacetedVisualizerLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

///////////////////////////////////////////////////////////////////////////////////////
// Helper methods
///////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
void vtkSlicerFacetedVisualizerLogic::toLower(std::string origstr, std::string& newstr)
{
   std::locale loc;
   newstr = "";
   for (unsigned int i = 0; i < origstr.length(); i++)
   {
#ifdef WIN32
      newstr += std::tolower(origstr[i]);
#else
	  newstr += std::tolower(origstr[i],loc);
#endif
	   
   }
}

//---------------------------------------------------------------------------
void vtkSlicerFacetedVisualizerLogic::toUpper(std::string origstr, std::string& newstr)
{
   std::locale loc;
   newstr = "";
   for (unsigned int i = 0; i < origstr.length(); i++)
   {
#ifdef WIN32
      newstr += std::toupper(origstr[i]);
#else
	  newstr += std::toupper(origstr[i],loc);
#endif
   }
}

//---------------------------------------------------------------------------
void vtkSlicerFacetedVisualizerLogic::removeLeadingFollowingSpace(std::string &origstr)
{

	  size_t pos = origstr.find_first_of(" ");
	  while(pos != std::string::npos && pos < 1)
	  {
	    origstr = origstr.substr(pos+1);
	    pos = origstr.find_first_of(" ");
	  }
	  unsigned len = origstr.length();
	  pos = origstr.find_last_of(" ");
	  while(pos != std::string::npos && pos > len - 2)
	  {
	    origstr = origstr.substr(0, pos);
	    pos = origstr.find_last_of(" ");
	    len = origstr.length();
	  }

}

//---------------------------------------------------------------------------
int vtkSlicerFacetedVisualizerLogic::
AddQueryResult(std::string &text, std::vector< std::string >& store)
{

	bool found = false;
	int index = -1;
	for (unsigned n = 0; !found && n < store.size(); ++n)
	{
		found = store[n] == text; //std::strcmp(store[n],text) == 0;
		index = n;
	}
	if(!found)
	{
		store.push_back(text);
		index = store.size()-1;
	}
	return index;
}


//---------------------------------------------------------------------------
std::string vtkSlicerFacetedVisualizerLogic::GetDBSubject(std::string &query,
		vtk_sqlite3* ptrDB)
{

	char *errmsg;
	char **currResult;
	int nrows, ncols;
	std::string Subject = query;

	char *sqlQuery = ProduceQuery(Subject, false, true,"");


	vtk_sqlite3_get_table(ptrDB, sqlQuery, &currResult, &nrows, &ncols, &errmsg);
	if(nrows <= 0)
	{
		// check if we have an equivalent query term
		std::map<std::string, std::string>::iterator rEq = eqQueryMap.find(query);
		if(rEq != eqQueryMap.end())
		{
			Subject = (*rEq).second;

		}
		else
		{
			sqlQuery = ProduceQuery(query, true, false, "non_english_equivalent");
			int status = vtk_sqlite3_get_table(ptrDB, sqlQuery, &currResult, &nrows, &ncols, &errmsg);
			if(nrows <= 0)
			{
				sqlQuery = ProduceQuery(query, true, false, "synonym");
				status = vtk_sqlite3_get_table(ptrDB, sqlQuery, &currResult, &nrows, &ncols, &errmsg);
				if(nrows <=0)
				{
					return "null";
				}
				else
				{
					Subject = *(currResult+(ncols+0));
				}
			}
			else
			{
				Subject = *(currResult+(ncols+0));
			}
			eqQueryMap.insert(std::pair< std::string, std::string > (query, Subject));
		}
	}

	return Subject;
}



//---------------------------------------------------------------------------
void vtkSlicerFacetedVisualizerLogic
::GetQueryResults(std::vector< std::vector < std::string > > &results, std::vector< std::string > &queries)
{

	for (unsigned n = 0; n < resultsForDisplay.size(); ++n)
	{
		size_t pos = resultsForDisplay[n].find(";");
		std::string q = resultsForDisplay[n].substr(0, pos);
		unsigned int index = this->AddQueryResult(q, queries);
		if(index == results.size())
		{
			std::vector< std::string > newresults;
			newresults.push_back(resultsForDisplay[n].substr(pos+1));
			results.push_back(newresults);
		}
		else
		{
			results[index].push_back(resultsForDisplay[n].substr(pos+1));
		}
	}
}


//---------------------------------------------------------------------------
char* vtkSlicerFacetedVisualizerLogic::ProduceQuery(std::string& string,
		                               bool asObject,
		                               bool asSubject,
		                               std::string Predicate)
{

	// convert string to DB form
    size_t pos = string.find_first_of("'");
	if(pos != std::string::npos)
	{
	  string = string.substr(pos+1);
	}
	pos = string.find_last_of("'");
	if(pos != std::string::npos)
	{
       string = string.substr(0, pos);
	}
	// remove any paranthesis
	pos = string.find_first_of("(");
	if(pos != std::string::npos)
	{
	   string = string.substr(pos+1);
	}
	pos = string.find_last_of(")");
	if(pos != std::string::npos)
	{
	  string = string.substr(0, pos);
	}
	string[0] = std::toupper(string[0]);
	pos = string.find_first_of(" ");
	while(pos != std::string::npos)
	{
	   std::string leadstr = string.substr(0, pos);
	   std::string trailstr = string.substr(pos+1);
	   string = leadstr + "_" + trailstr;
	   pos = string.find_first_of(" ");
	}

	 //if(std::strcmp(Predicate.c_str(),"")==0)
	if(Predicate == "")
	{
	   if(!asObject && !asSubject)
	   {
	      return (vtk_sqlite3_mprintf("SELECT * from resources where subject = '%q' or object = '%q'",
	        		string.c_str(), string.c_str()));
	   }
	   else if(asObject)
	   {
		   return (vtk_sqlite3_mprintf("SELECT * from resources where object = '%q'", string.c_str()));
	   }
	   else
	   {
  		  return (vtk_sqlite3_mprintf("SELECT * from resources where subject = '%q'", string.c_str()));
	   }
	 }
	 else
	 {
		 if(!asObject && !asSubject)
		   {
			  return (vtk_sqlite3_mprintf("SELECT * from resources where (subject = '%q' or object = '%q') and predicate = '%q'",
					  string.c_str(), string.c_str(), Predicate.c_str()));
		   }
		   else if(asObject)
		   {
			   return (vtk_sqlite3_mprintf("SELECT * from resources where object = '%q' and predicate = '%q'",
					   string.c_str(), Predicate.c_str()));
		   }
		   else
		   {
			  return (vtk_sqlite3_mprintf("SELECT * from resources where subject = '%q' and predicate = '%q'",
					  string.c_str(), Predicate.c_str()));
		   }
	 }

}


//----------------------------------------------------------------------------------
// syncs a given model with the Database. Checks if the model can be found in the DB

void vtkSlicerFacetedVisualizerLogic::SyncModelWithDB(vtkMRMLModelHierarchyNode *modelNode,
		vtk_sqlite3* ptrDB, std::vector< std::string > &possibleMatchingDBEntries)
{

	// Following is to fix working with the Abdominal atlas
	std::string modelName = modelNode->GetName();
	std::string lomodelName;
	this->toLower(modelName, lomodelName);
	size_t posCheck = lomodelName.find("vtkmrmlmodelhierarchynode");
	if(posCheck != std::string::npos)
	{
		modelName = vtkMRMLModelNode::SafeDownCast(modelNode->GetAssociatedNode())->GetName();
		std::cout<<" using mrmlmodelNode instead of hierarchy node "<<modelName<<std::endl;
		this->toLower(modelName, lomodelName);
		posCheck = lomodelName.find("_");
		if(posCheck != std::string::npos)
		{
			lomodelName = lomodelName.substr(posCheck+1);
			posCheck = lomodelName.find_first_of("_");
			lomodelName = lomodelName.substr(posCheck+1);
		}
	}

	// convert the model name string in to the correct form for use in the DB.
	// Currently we assume that in the DB, the atoms are represented as "White_matter_of_cerebellum"
	// following the way the atoms are represented in the FMA ontology
	std::string string = lomodelName;
	// convert string to DB form
	size_t pos = string.find_first_of("'");
	if(pos != std::string::npos)
	{
	  string = string.substr(pos+1);
	}
	pos = string.find_last_of("'");
	if(pos != std::string::npos)
	{
	   string = string.substr(0, pos);
	}
	// remove any paranthesis
	pos = string.find_first_of("(");
	if(pos != std::string::npos)
	{
	   string = string.substr(pos+1);
	}
	pos = string.find_last_of(")");
	if(pos != std::string::npos)
	{
	  string = string.substr(0, pos);
	}
	string[0] = std::toupper(string[0]);

	std::vector< std::string > individualStrings;
	int countNonWildCardStrings = 0;
	pos = string.find_first_of(" ");

	// the following gets rid of non-useful search strings to break down a complex string into
	// individual strings so we can match the model to the DB using individual string combinations
	// Ex: "White_matter_of_cerebellum" as "White_matter%", "%matter%cerebellum", etc.

	while(pos != std::string::npos)
	{
	   std::string leadstr = string.substr(0, pos);
	   std::string trailstr = string.substr(pos+1);
	   size_t p = leadstr.find_last_of("_");
	   std::string tmpstr = leadstr;
	   if( p != std::string::npos)
	   {
		   tmpstr = leadstr.substr(p+1);
	   }

	   std::cout<<" leadstr "<<tmpstr<<std::endl;

	   bool foundIgnore = std::strcmp(tmpstr.c_str(), "of") == 0;
	   if(!foundIgnore)
	   {
		 individualStrings.push_back(tmpstr);
	   }

	   string = leadstr + "_" + trailstr;
	   pos = string.find_first_of(" ");

	   if(pos == std::string::npos)
	   {
		   foundIgnore = std::strcmp(trailstr.c_str(), "of") == 0;
		   if(!foundIgnore)
		   {
			   individualStrings.push_back(trailstr);
		   }
	   }
	   countNonWildCardStrings = individualStrings.size();
	}

	modelName = string;

   // convert text to DB form
   char *query = this->ProduceQuery(modelName, false, false,"");
   std::cout<<" Synching model "<<modelName<<" with DB and \n query "<<query<<std::endl;
   // confirm that the query occurs in the DB
   char *errmsg;
   char **currResult;
   int nrows, ncols;

   // querying DB to test if the current node is present
   int status = vtk_sqlite3_get_table(ptrDB, query, &currResult, &nrows, &ncols, &errmsg);

   if(nrows > 0)
   {
	   // parse the query string into individual parts
	   std::cout<<" inserting into modelDB pair "<<modelName<<" : "<<modelNode->GetName()<<std::endl;
	   mrmlDBTerms.insert(std::pair< std::string, std::string > (this->GetDBSubject(modelName, ptrDB), modelNode->GetName()));
   }
   else if(individualStrings.size() > 0)
   {

	   bool foundInDB = false;
	   unsigned int count = 1;
	   while(!foundInDB && count < individualStrings.size()-1)
	   {
		   std::string str2 = "";
		   for(int s = individualStrings.size()-count-1; s >=0; s--)
		   {
			   str2 = individualStrings[s] + "%" + str2;
		   }
		   std::string str1 = "";
		   for (unsigned int s = count; s < individualStrings.size(); s++)
		   {
			   str1 += "%" + individualStrings[s];

		   }
		   ++count;
		   if(individualStrings.size() == 2)
		   {
		     std::cout<<" trying to match "<<modelName<<" with new strings "<<str1<<"  & "<<str2<<std::endl;
		   }
		   char *newquery = vtk_sqlite3_mprintf("select * from resources where subject like '%q' or subject like '%q'",
				   str1.c_str(), str2.c_str());

		   status = vtk_sqlite3_get_table(ptrDB, newquery, &currResult, &nrows, &ncols, &errmsg);
		   if(individualStrings.size() == 2)
		   {
		     std::cout<<"number of results from SQL query "<<nrows<<std::endl;
		   }
		   if(nrows > 0)
		   {
			   foundInDB = true;

			   for (int nr = 1; nr < nrows; ++nr)
			   {
				   std::string tmpstr = *(currResult+(nr*ncols));
				   this->AddQueryResult(tmpstr, possibleMatchingDBEntries);
			   }
		   }
	   }
	   if(!foundInDB)
	   {
		   // Add the node as a local non-DB node
		   std::string modelName = modelNode->GetName();
		   this->AddQueryResult(modelName, this->nonDBElements);
	   }
   }

}

//------------------------------------------------------------------------------------
void vtkSlicerFacetedVisualizerLogic
::SynchronizeAtlasWithDB(std::vector< std::vector< std::string > > &matchingDBAtoms,
		std::vector< std::string > &unMatchedMRMLAtoms)
{
   

   std::vector< std::string > DBModelNodes;
   // get the models in the atlas
   vtk_sqlite3 *ptrDB;
   int status = vtk_sqlite3_open(dbFileName.c_str(), &ptrDB);
   if(status != 0)
   {
	   this->setValidDBFileName = false;
     unsigned int nmodelNodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLModelNode");
     std::cout<<" Number Model Nodes "<<nmodelNodes<<std::endl;
     // the first three are red, yellow and green slices
     for (unsigned int n = 0; n < nmodelNodes; ++n)
     {
       vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(
           this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLModelNode"));

       std::string modelName = modelNode->GetName();
       this->AddQueryResult(modelName, this->nonDBElements);
     }
     return;
   }
   else
   {
      this->setValidDBFileName = true;
   }
 
   unsigned int nmodels = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLModelHierarchyNode");

   for (unsigned int n = 0; n < nmodels; n++)
   {
	  // get model name and check if the corresponding text exists in the database
       vtkMRMLModelHierarchyNode *modelNode = vtkMRMLModelHierarchyNode::SafeDownCast(
    		   this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLModelHierarchyNode"));

       if(modelNode->GetNumberOfChildrenNodes() == 0)
       {
    	   std::string nodeID = modelNode->GetAssociatedNodeID();
    	   this->AddQueryResult(nodeID, DBModelNodes);
       }
       else
       {
		   vtkSmartPointer<vtkCollection> c = vtkSmartPointer<vtkCollection>::New();

		   modelNode->GetChildrenModelNodes(c);

		   for (int t = 0; t < c->GetNumberOfItems(); ++t)
		   {
			  vtkMRMLModelNode *node = vtkMRMLModelNode::SafeDownCast(c->GetItemAsObject(t));
			  std::string nodeID = node->GetID();
			  this->AddQueryResult(nodeID, DBModelNodes);
		   }
       }
       std::vector< std::string > possibleMatchingEntries;
       this->SyncModelWithDB(modelNode, ptrDB, possibleMatchingEntries);
       std::string modelName = modelNode->GetName();
       if(possibleMatchingEntries.size() > 0)
       {
         matchingDBAtoms.push_back(possibleMatchingEntries);
         unMatchedMRMLAtoms.push_back(modelName);
       }
   }

   // close the database
    vtk_sqlite3_close(ptrDB);

   // get all the model nodes and check if they are already used by the hierarchy nodes.
   // otherwise we just add it as a non-DB node and use it directly for displaying when the appropriate
   // user query is encountered. Useful for displaying user added models to the scene

   unsigned int nmodelNodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLModelNode");
   std::cout<<" Number Model Nodes "<<nmodelNodes<<std::endl;
   // the first three are red, yellow and green slices
   for (unsigned int n = 0; n < nmodelNodes; ++n)
   {
	   vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(
			   this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLModelNode"));

	   std::string modelName = modelNode->GetName();
	   std::string modelID = modelNode->GetID();

	   bool foundNode = false;
	   for (unsigned m = 0; !foundNode && m < DBModelNodes.size(); ++m)
	   {
		   foundNode = DBModelNodes[m] == modelID;
	   }
	   if(!foundNode)
	   {
	      this->AddQueryResult(modelName, this->nonDBElements);
	   }
   }

   // print out the Non DB nodes for debugging
   std::cout<<" num non DB elements "<<this->nonDBElements.size()<<std::endl;
   for (unsigned k = 0; k < this->nonDBElements.size(); ++k)
   {
	   std::cout<<" "<<this->nonDBElements[k]<<std::endl;
   }

}

//------------------------------------------------------------------------------------
int vtkSlicerFacetedVisualizerLogic::RecursiveProcessQuery(char* sqlquery,
		                              vtk_sqlite3* ptrDB,
		                              bool queryAsSubject,
		                              std::vector< std::string > &displayTerms)
{

	char *errmsg;
	char **currResult;
	int nrows, ncols;

	vtk_sqlite3_get_table(ptrDB, sqlquery, &currResult, &nrows, &ncols, &errmsg);
	std::vector< std::string > recursionSubjects;

	if(nrows > 0)
	{
		std::cout<<" number of row results "<<nrows<<" for query "<<sqlquery<<std::endl;
	}
	if(nrows <= 0)
	{
		return -1;
	}

	std::string query = sqlquery;

	size_t p1 = query.find(" SUBJECT = ");

	std::string querySubject = "";
	if(p1 != std::string::npos)
	{
		querySubject = query.substr(p1+12);
		size_t p2 = querySubject.find(" and OBJECT = ");
		if(p2 != std::string::npos)
		{
			querySubject = querySubject.substr(0, p2-1);
		}
	}

	if(nrows > 0)
	{

		for (int nr = 1; nr <= nrows; nr++)
		{

			std::string rowResult;
			bool displayResult = false;
			bool DontAddToResults = false;
			std::vector< std::string> term;
			term.resize(3);
			for (int nc = 0; nc < ncols; ++nc)
			{
				term[nc] = *(currResult+(nr*ncols+nc));
			}

			std::pair< std::multimap< std::string, std::string >::iterator,
			          std::multimap< std::string, std::string > ::iterator > mrmlIt;
			if(!queryAsSubject)
			{
			  mrmlIt = mrmlDBTerms.equal_range(this->GetDBSubject(term[0], ptrDB)); //mrmlDBTerms.find(this->GetDBSubject(term[0], ptrDB));
			}
			else
			{
				mrmlIt = mrmlDBTerms.equal_range(this->GetDBSubject(term[0], ptrDB));//mrmlDBTerms.find(this->GetDBSubject(term[2], ptrDB));
			}
			std::multimap< std::string, std::string >::iterator itr1 = mrmlIt.first;
			std::multimap< std::string, std::string >::iterator itr2 = mrmlIt.second;

			if(itr1 != mrmlDBTerms.end())
			{
				std::multimap< std::string, std::string >::iterator itMRML;
				for(itMRML = itr1; itMRML != itr2; ++itMRML)
				{
					this->AddQueryResult(itMRML->second, displayTerms);
				}
				//std::cout<<" row result: "<<term[0]<<", "<<term[1]<<", "<<term[2]<<std::endl;
				//this->AddQueryResult(mrmlIt->second, displayTerms);
				//displayResult = true;
			}


			if(!displayResult && !DontAddToResults)
			{
				// check if the object term needs to be recursed. We need recursion only when we don't have
				// display nodes attached to it
				bool foundDisplayNode = false;
				std::pair< std::multimap< std::string, int>::iterator,
					          std::multimap< std::string, int > ::iterator> ret;
				if(queryAsSubject)
				{
					ret =queryCacheMap.equal_range(term[2]);
				}
				else
				{
					ret = queryCacheMap.equal_range(term[1]);
				}

				std::multimap< std::string, int >::iterator itrF = ret.first;
				std::multimap< std::string, int >::iterator itrS = ret.second;


				if(itrF != queryCacheMap.end())
				{
					std::multimap<std::string, int > ::iterator itr;
					for(itr = ret.first; itr != ret.second; ++itr)
					{
						std::string t = queryResultCache[(*itr).second];
						size_t p1 = t.find(";");
						size_t p2 = t.find_last_of(";");
						std::string predicate = t.substr(p1+1, p2);
						if(!foundDisplayNode)
						{
							foundDisplayNode = predicate == "mrmlName"; //std::strcmp(predicate.c_str(),"mrmlName") == 0;
						}
					}
				}
				if(!foundDisplayNode)
				{
					if(queryAsSubject)
					{
						recursionSubjects.push_back(term[2]);
					}
					else
					{
						recursionSubjects.push_back(term[1]);
					}
				}

			}
		} // end for
	}

	std::cout<<" number of recursion subjects :"<<recursionSubjects.size()<<std::endl;
	// recursive query on the subject terms
	for (unsigned ns = 0; ns < recursionSubjects.size(); ns++)
	{
		for (unsigned nrec = 0; nrec < recursionPredicates.size(); ++nrec)
		{
			char *sqlQuery = this->ProduceQuery(recursionSubjects[ns], false, true, recursionPredicates[nrec]);
			RecursiveProcessQuery(sqlQuery, ptrDB, true, displayTerms);
		}

		for (unsigned nrec = 0; nrec < addRecursionPredicates.size(); ++nrec)
		{
			char *sqlQuery = this->ProduceQuery(recursionSubjects[ns], true, false, addRecursionPredicates[nrec]);
			RecursiveProcessQuery(sqlQuery, ptrDB, false, displayTerms);
		}

	}
	return 0;
}


//----------------------------------------------------------------------------------------------
int vtkSlicerFacetedVisualizerLogic::ProcessSingleQuery(std::string& query, vtk_sqlite3* ptrDB,
		std::vector< std::string > &queryResults, std::vector< std::string > &displayTerms)
{

	// first check if its a two-part query
	size_t pos = query.find(";");
	bool twoPartQuery = pos != std::string::npos;
	std::string firstPart = query;
	std::string secondPart = "";
	std::cout<<" query is "<<query<<std::endl;
	if(twoPartQuery)
	{
	  firstPart = query.substr(0, pos);
	  secondPart = query.substr(pos+1);

	}
	else
	{
		//std::map< std::string, std::string > ::iterator mrmlIt = mrmlDBTerms.find(this->GetDBSubject(firstPart, ptrDB));
		std::pair< std::multimap< std::string, std::string >::iterator,
					          std::multimap< std::string, std::string > ::iterator > mrmlIt;
		mrmlIt = mrmlDBTerms.equal_range(this->GetDBSubject(firstPart, ptrDB));
		std::multimap< std::string, std::string >::iterator itr1 = mrmlIt.first;
		std::multimap<std::string, std::string>::iterator itr2 = mrmlIt.second;

		if(itr1 != mrmlDBTerms.end())
		{
			std::multimap<std::string, std::string>::iterator itr3;
			for(itr3 = itr1; itr3 != itr2; ++itr3)
			{
				this->AddQueryResult(itr3->second, displayTerms);
			}
		}
	}

	if(!twoPartQuery)
	{
		// search the local db for non-DataBase queries (Examples: "mass", "tumor" models added to the scene by the user
		bool found = false;
		int indx = 0;
		for (unsigned int l = 0; !found && l < nonDBElements.size(); l++)
		{
			std::string lqdb;
			std::string lqQ;
			this->toLower(nonDBElements[l], lqdb);
			this->toLower(query, lqQ);
			found = lqdb == lqQ;
			indx = l;
		}
		if(found)
		{
			displayTerms.push_back(nonDBElements[indx]);
			return 0;
		}
	}
  if(!this->setValidDBFileName)
  {
    return -1;
  }
	// check if the query is in the cache
	// Cache logic commented out as it is not functionality is not fully tested. HV
//	std::pair< std::multimap< std::string, int>::iterator,
//	          std::multimap< std::string, int > ::iterator> ret =
//	                        queryCacheMap.equal_range(firstPart);
//	std::multimap< std::string, int >::iterator itr = ret.first;
//	if(itr == queryCacheMap.end())
//	{
//		// check if the equivalent query is present
//		std::map< std::string, std::string > ::iterator iteq = eqQueryMap.find(firstPart);
//		if(iteq != eqQueryMap.end())
//		{
//			ret = queryCacheMap.equal_range((*iteq).second);
//		}
//
//	}


	// if there is a second part to the query we need a more refined search
	if(secondPart != "")
	{
		char *sqlQuery;
//		itr = ret.first;
//		if(itr != queryCacheMap.end())
//		{
//			std::cout<<" found query in cache "<<std::endl;
//			//std::multimap<std::string, int > ::iterator itr;
//			bool foundSecondInCache = false;
//
//			for(itr = ret.first; itr != ret.second; ++itr)
//			{
//				std::string term = queryResultCache[(*itr).second];
//				size_t p1 = term.find(";");
//				size_t p2 = term.find_last_of(";");
//				std::string predicate = term.substr(p1+1, p2-1);
//
//				foundSecondInCache = predicate == secondPart; //(std::strcmp(predicate,secondPart) == 0);
//				if(foundSecondInCache)
//			    {
//					std::cout<<" adding term to results : "<<term<<std::endl;
//				   //this->AddQueryResult(term, queryResults);
//				   std::string tmpstr = term.substr(p2+1);
//				   this->AddQueryResult(tmpstr, displayTerms);
//				}
//			}
//			if(!foundSecondInCache)
//			{
//				// we need to search for the query in the DB
//				std::string subject = this->GetDBSubject(firstPart, ptrDB);
//				if(subject == "null")
//				{
//					return -1;
//				}
//				sqlQuery = ProduceQuery(subject, false, true, secondPart);
//				//std::cout<<" producing sql query with subject "<<subject<<" predicate "<<secondPart<<std::endl;
//				//std::cout<<" sql Query is "<<sqlQuery<<std::endl;
//				RecursiveProcessQuery(sqlQuery, ptrDB, true, displayTerms);
//			}
//		}
//		else
//		{
			std::string subject = this->GetDBSubject(firstPart, ptrDB);
			if(subject == "null")
			{
				return -1;
			}
			sqlQuery = ProduceQuery(subject, false, true, secondPart);
			//std::cout<<" SQL query is "<<sqlQuery<<" with SUBJECT :"<<subject<<" and PREDICATE : "<<secondPart<<std::endl;
			RecursiveProcessQuery(sqlQuery, ptrDB, true, displayTerms);
//		}
		char *errmsg;
		char **currResult;
		int nrows, ncols;
		vtk_sqlite3_get_table(ptrDB, sqlQuery, &currResult, &nrows, &ncols, &errmsg);
		if(nrows <= 0)
		{
			return -1;
		}
		std::vector< std::string > relations;
		for (int nr = 1; nr <= nrows; ++nr)
		{
			std::vector< std::string> term;
			term.resize(3);
			for (int nc = 0; nc < ncols; ++nc)
			{
				term[nc] = *(currResult+(nr*ncols+nc));
			}
			// test if this is a ignore predicate
			bool addToResults = true;

			for (unsigned np= 0; addToResults && np < ignorePredicates.size(); ++np)
			{
				std::string lstr;
				this->toLower(term[1], lstr);
				addToResults = ignorePredicates[np] == lstr ? false : true; //std::strcmp(ignorePredicates,lstr) == 0;
			}
			if(addToResults)
			{
				// check if this is a recursion Predicate. In this case we keep the whole term for display
				bool isRecursionPredicate = false;
				for (unsigned c = 0; !isRecursionPredicate && c < recursionPredicates.size(); ++c)
				{
					isRecursionPredicate = recursionPredicates[c] == term[1] && term[1] != secondPart;
				}
				for (unsigned c = 0; !isRecursionPredicate && c < addRecursionPredicates.size(); ++c)
				{
					isRecursionPredicate = addRecursionPredicates[c] == term[1];
				}
				if(!isRecursionPredicate)
				{
					if(term[1] == secondPart)
					{
						std::string tmpstr = term[0]+";"+term[2];
						this->AddQueryResult(tmpstr, queryResults);
					}
						// check if its a comment predicate
					bool isCommentPredicate = false;
					for (unsigned c = 0; !isCommentPredicate && c < commentPredicates.size(); ++c)
					{
						isCommentPredicate = commentPredicates[c] == term[1];
					}
					if(isCommentPredicate)
					{
						std::string tmpstr = term[0] +";comment;"+term[2];
						this->AddQueryResult(tmpstr, queryResults);
					}

				}
			}
		}

	}
	else
	{
	   // there is no second part to the query
	   // for display, we only need to check the recursion predicates
	   std::string subject = this->GetDBSubject(firstPart, ptrDB);
	   for (unsigned n = 0; n < recursionPredicates.size(); ++n)
	   {
		    std::string tmpstr = subject+";"+recursionPredicates[n];
		    std::cout<<" re-process as two-part query "<<tmpstr<<std::endl;
			ProcessSingleQuery(tmpstr, ptrDB, queryResults, displayTerms);
	   }
	   for (unsigned n = 0; n < addRecursionPredicates.size(); ++n)
	   {
		   char *sqlQuery = ProduceQuery(subject, true, false, addRecursionPredicates[n]);
		   RecursiveProcessQuery(sqlQuery, ptrDB, false, displayTerms);
	   }
		// get all the predicates related to this query from the DB without recursion
		char *sqlQuery = ProduceQuery(subject, false, true,"");
		char *errmsg;
		char **currResult;
		int nrows, ncols;
		vtk_sqlite3_get_table(ptrDB, sqlQuery, &currResult, &nrows, &ncols, &errmsg);
		if(nrows <= 0)
		{
			return -1;
		}
		std::vector< std::string > relations;
		for (int nr = 1; nr <= nrows; ++nr)
		{
			std::vector< std::string> term;
			term.resize(3);
			for (int nc = 0; nc < ncols; ++nc)
			{
				term[nc] = *(currResult+(nr*ncols+nc));
			}
			// test if this is a ignore predicate
			bool addToResults = true;

			for (unsigned np= 0; addToResults && np < ignorePredicates.size(); ++np)
			{
				std::string lstr;
				this->toLower(term[1], lstr);
				addToResults = ignorePredicates[np] == lstr ? false : true; //std::strcmp(ignorePredicates,lstr) == 0;
			}
			if(addToResults)
			{
				// check if this is a recursion Predicate. In this case we keep the whole term for display
				bool isRecursionPredicate = false;
				for (unsigned c = 0; !isRecursionPredicate && c < recursionPredicates.size(); ++c)
				{
					isRecursionPredicate = recursionPredicates[c] == term[1];
				}
				for (unsigned c = 0; !isRecursionPredicate && c < addRecursionPredicates.size(); ++c)
				{
					isRecursionPredicate = addRecursionPredicates[c] == term[1];
				}
				if(!isRecursionPredicate)
				{
						// check if its a comment predicate
					bool isCommentPredicate = false;
					for (unsigned c = 0; !isCommentPredicate && c < commentPredicates.size(); ++c)
					{
						isCommentPredicate = commentPredicates[c] == term[1];
					}
					if(isCommentPredicate)
					{
						std::string tmpstr = term[0] +";comment;"+term[2];
						this->AddQueryResult(tmpstr, queryResults);
					}
					else
					{
						std::string tmpstr = term[0] + ";" + term[1];
						this->AddQueryResult(tmpstr, relations);
					}
				}
			}
		}
		// now construct terms using the relations and add to queryResult
		for (unsigned r = 0; r < relations.size(); r++)
		{
			this->AddQueryResult(relations[r], queryResults);
		}
	}

	std::cout<<" query "<<firstPart<<"; secondPart "<<queryResults.size()<<std::endl;

	return 0;
}

//-----------------------------------------------------------------------------------------
// Currently not used due to lack of full testing. -- HV
int vtkSlicerFacetedVisualizerLogic::
ManageQueryCache(std::vector< std::string > &queries, std::string query , std::vector< std::string > & queryResults)
{
	int spaceNeeded = queryResults.size();
	int spaceRemaining = cacheSize - queryResultCache.size();
	if(spaceRemaining >= spaceNeeded)
	{
		queryResultAge.insert(std::pair< std::string, int > (query, 0));
		for(int n = 0; n < spaceNeeded; ++n)
		{
			queryResultCache.push_back(queryResults[n]);

			queryCacheMap.insert(std::pair< std::string, int >(query, queryResultCache.size()-1));
		}
	}
	else
	{
		int newSpace = 0;
		bool queryFoundForCleanup = true;
		std::vector< unsigned int > queryResultIndices;
		while(newSpace < spaceNeeded && queryFoundForCleanup)
		{
			// need to clean up cache for the new query
			queryFoundForCleanup = false;
			int oldestQueryAge = 0;
			std::string oldestQuery ="";
			for (std::map<std::string, int > ::iterator it = queryResultAge.begin();
					it != queryResultAge.end(); ++it)
			{
				if(it->second < 2)
				{
					continue;
				}
				bool found = false;
				for (unsigned n = 0; !found && n < queries.size(); ++n)
				{
					if(queries[n] == it->first) // std::strcmp(queries[n],(*it).first) == 0)
					{
						it->second = it->second - 1;
					}
					else
					{
						if(oldestQueryAge < it->second)
						{
							oldestQueryAge = it->second;
							oldestQuery = it->first;
							queryFoundForCleanup = true;
						}
					}
				}
			}
			if(queryFoundForCleanup)
			{
				// find the corresponding results for this query and create space in those indices
				std::pair< std::multimap< std::string, int >::iterator,
					      std::multimap< std::string, int >::iterator > r =
					    		  queryCacheMap.equal_range(oldestQuery);

				std::multimap<std::string, int > ::iterator itr = r.first;
				if(itr != queryCacheMap.end())
				{

					//std::multimap<std::string, int > ::iterator itr;
					for (itr = r.first; itr != r.second; ++itr)
					{
						queryResultIndices.push_back((*itr).second);
					}
					queryCacheMap.erase(r.first, r.second);
				}
				// cleanup
				queryResultAge.erase(oldestQuery);
				queryCacheMap.erase(oldestQuery);
				newSpace = queryResultIndices.size();
			}
		}
		std::vector< std::string > tempQueryResults;
		for (unsigned int r = 0; r < queryResultCache.size(); ++r)
		{
			bool found = false;
			for (unsigned j = 0; j < queryResultIndices.size(); ++j)
			{
				found = queryResultIndices[j] == r;
			}
			if(!found)
			{
				tempQueryResults.push_back(queryResultCache[r]);
			}
		}
		queryResultCache.swap(tempQueryResults);
		if(newSpace >= spaceNeeded)
		{
			queryResultAge.insert(std::pair< std::string, int > (query, 0));
			for(int n = 0; n < spaceNeeded; ++n)
			{
				queryResultCache.push_back(queryResults[n]);

				queryCacheMap.insert(std::pair< std::string, int >(query, queryResultCache.size()-1));
			}
		}
	}

	return (cacheSize - queryResultCache.size());

}


// to do: need to deal with non-DB queries such as "tumor", "mass" coming from user segmented
// models added to the scene.
//-----------------------------------------------------------------------------------------
bool vtkSlicerFacetedVisualizerLogic
::ProcessQuery()
{

	for(std::map<std::string,int>::iterator it = queryResultAge.begin();
				it != queryResultAge.end(); ++it)
	{
		it->second = it->second + 1;
	}

    // construct a query for the database
	vtk_sqlite3 *ptrDB;
	vtk_sqlite3_open(dbFileName.c_str(), &ptrDB);

	resultsForDisplay.clear();

	std::vector< std::string > queries;

	bool isSimpleQuery = false;
    // first split the query if this is a complex query
    size_t foundPos = this->query.find("+");
    if(foundPos == std::string::npos)
    {
    	foundPos = this->query.find(",");
    	isSimpleQuery = (foundPos == std::string::npos);
    }
    std::string qstring = query;
    if(!isSimpleQuery)
    {
    	while(foundPos != std::string::npos)
    	{
    		std::string q = qstring.substr(0, foundPos);
    		std::string lq;
    		this->toLower(q, lq);
    		this->removeLeadingFollowingSpace(lq);
    		queries.push_back(lq);
    		qstring = qstring.substr(foundPos+1);
    		foundPos = qstring.find("+");
    		if(foundPos == std::string::npos)
    		{
    			foundPos = qstring.find(",");
    		}
    		if(foundPos == std::string::npos)
    		{
    			this->toLower(qstring, lq);
    			this->removeLeadingFollowingSpace(lq);
    			queries.push_back(lq);
    		}
        }
    }
    else
    {
    	std::string lowerQ;
    	this->toLower(query, lowerQ);
    	queries.push_back(lowerQ);
    }

    // first check if we already know how to display the query
    // If yes, lets just add a "more info" branch to the query and continue
    // add "click more info to get more information about query" in the comment box

    //std::vector< std::vector< std::string > > allqueryResults;
    //int cacheStatus = 0;
    std::vector< std::string > queryDisplayResults;


	for (unsigned n = 0; n < queries.size(); n++)
    {
		std::vector< std::string > queryResults;
		std::vector< std::string > cacheResults;
		std::string q = queries[n];

		size_t pos = q.find(";");
		if(pos != std::string::npos && this->setValidDBFileName)
		{
			std::string firstPart = q.substr(0, pos);
			std::string secondPart = q.substr(pos+1);
			char *errmsg;
			char **qResult;
			int numrows, numcols;
			char *squery = this->ProduceQuery(secondPart, false, true, "");
			vtk_sqlite3_get_table(ptrDB, squery, &qResult, &numrows, &numcols, &errmsg);
			if(numrows > 0)
			{
				q = secondPart;
				queries[n] = secondPart;
			}
		}
		std::vector< std::string > displayTerms;
		int status = ProcessSingleQuery(q, ptrDB, queryResults, displayTerms);

		for (unsigned i = 0; i < queryResults.size(); ++i)
		{
			size_t pos = queryResults[i].find(";");
			size_t p1 = q.find(";");
			if(p1 != std::string::npos)
			{
			   std::string tmpstr = q.substr(0,p1)+"-"+q.substr(p1+1)+queryResults[i].substr(pos);
			   resultsForDisplay.push_back(tmpstr);
			}
			else
			{
			  std::string tmpstr = q+queryResults[i].substr(pos);
			  resultsForDisplay.push_back(tmpstr);
			}
		}

		std::cout<<" number of display terms "<<displayTerms.size()<<std::endl;
		if(status == 0)
		{
			//allqueryResults.push_back(queryResults);

			for (unsigned d = 0; d < displayTerms.size(); d++)
			{

			   this->AddQueryResult(displayTerms[d], queryDisplayResults);
			   // test if query is a simple or two-part query
			   size_t p = q.find(";");
			   if(p == std::string::npos)
			   {
				   std::string tmpstr = q+";"+recursionPredicates[0]+";"+displayTerms[d];
				   this->AddQueryResult(tmpstr, cacheResults);
			   }
			   else
			   {
				   std::string tmpstr = q+";"+displayTerms[d];
				   this->AddQueryResult(tmpstr, cacheResults);
			   }
			}
//			if(cacheStatus >= 0)
//			{
//			  cacheStatus = this->ManageQueryCache(queries, q, cacheResults);
//			}
		}
    }

	std::cout<<" query results "<<std::endl;
	std::vector< std::vector< std::string > > qResults;
	std::vector< std::string > allQueries;
	this->GetQueryResults(qResults, allQueries);
	for (unsigned n = 0; n < allQueries.size(); n++)
	{
		std::cout<<"-"<<allQueries[n]<<std::endl;

		for (unsigned j = 0; j < qResults[n].size(); ++j)
		{
			std::cout<<"---"<<qResults[n][j]<<std::endl;
		}

	}

	vtk_sqlite3_close(ptrDB);


	// display the results on the scene
	// remove old displays from the scene and add the new display
	unsigned nmodels = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLModelHierarchyNode");
	for (unsigned n = 0; n < nmodels; n++)
	{
	  // get model name and check if the corresponding text exists in the database
	  vtkMRMLModelHierarchyNode *modelNode = vtkMRMLModelHierarchyNode::SafeDownCast(
			                                  this->GetMRMLScene()->GetNthNodeByClass(
			                                  n, "vtkMRMLModelHierarchyNode"));
	  vtkSmartPointer<vtkCollection> c = vtkSmartPointer<vtkCollection>::New();

	  modelNode->GetChildrenModelNodes(c);
	  for (int t = 0; t < c->GetNumberOfItems(); ++t)
	  {
		vtkMRMLModelNode *node = vtkMRMLModelNode::SafeDownCast(c->GetItemAsObject(t));
		node->SetDisplayVisibility(0);
	  }
	}
	// disable display of all user nodes
	for (unsigned n = 0; n < this->nonDBElements.size(); ++n)
	{
		std::string name = nonDBElements[n];
		vtkSmartPointer<vtkCollection> mnodes = vtkSmartPointer<vtkCollection>::New();
				mnodes = this->GetMRMLScene()->GetNodesByClassByName("vtkMRMLModelNode",
						name.c_str());
		for(int j = 0; j < mnodes->GetNumberOfItems(); ++j)
		{
			vtkMRMLModelNode *node = vtkMRMLModelNode::SafeDownCast(mnodes->GetItemAsObject(j));
			std::string nodeName = node->GetName();
			if(name == nodeName)
			{
				node->SetDisplayVisibility(0);
			}
		}
	}


	for (unsigned j = 0; j < queryDisplayResults.size(); j++)
	{
		bool foundNode = false;
		//std::cout<<" display result "<<resultsForDisplay[j]<<std::endl;
		vtkSmartPointer<vtkCollection> mnodes = vtkSmartPointer<vtkCollection>::New();
		mnodes = this->GetMRMLScene()->GetNodesByClassByName("vtkMRMLModelHierarchyNode",
				queryDisplayResults[j].c_str());
		foundNode = mnodes->GetNumberOfItems() > 0;
		for(int j1 = 0; j1 < mnodes->GetNumberOfItems(); ++j1)
		{
			vtkMRMLModelHierarchyNode *mHNode = vtkMRMLModelHierarchyNode::SafeDownCast(mnodes->GetItemAsObject(j1));
			vtkSmartPointer< vtkCollection > cnodes = vtkSmartPointer< vtkCollection>::New();
			mHNode->GetChildrenModelNodes(cnodes);
			for (int t = 0; t < cnodes->GetNumberOfItems(); ++t)
			{
				vtkMRMLModelNode *node = vtkMRMLModelNode::SafeDownCast(cnodes->GetItemAsObject(t));
				node->SetDisplayVisibility(1);
			}
		}
		if(!foundNode)
		{
			vtkSmartPointer< vtkCollection> nodes = vtkSmartPointer<vtkCollection>::New();
			nodes = this->GetMRMLScene()->GetNodesByClassByName("vtkMRMLModelNode",
					queryDisplayResults[j].c_str());
			for (int j1 = 0; j1 < nodes->GetNumberOfItems(); ++j1)
			{
				vtkMRMLModelNode *node = vtkMRMLModelNode::SafeDownCast(nodes->GetItemAsObject(j1));
				node->SetDisplayVisibility(1);
			}
		}
	}

	return queryDisplayResults.size() > 0 ? true : false;


}


