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

#ifndef __qSlicerFacetedVisualizerModuleWidget_h
#define __qSlicerFacetedVisualizerModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerFacetedVisualizerModuleExport.h"

#include <list.h>
#include <string.h>
#include <vector.h>

class qSlicerFacetedVisualizerModuleWidgetPrivate;
class vtkMRMLNode;
class QString;
class QItemSelectionModel;
class QItemSelection;
class QStandardItemModel;

/// \ingroup Slicer_QtModules_FacetedVisualizer
class Q_SLICER_QTMODULES_FACETEDVISUALIZER_EXPORT qSlicerFacetedVisualizerModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerFacetedVisualizerModuleWidget(QWidget *parent=0);
  virtual ~qSlicerFacetedVisualizerModuleWidget();

public slots:

   void onOntologyFileName();
   void onQueryTextChanged(const QString &text);
   void onQuery();
   void onTreeItemSelected(const QItemSelection &, const QItemSelection &);
   void onQueryLogItemSelected(const QItemSelection &, const QItemSelection &);

   void onMatchingDBItemSelected(const QItemSelection&,
			const QItemSelection& );

   void onMatchDBMRMLAtom();

   void onMRMLAtomChanged(const QString &text);


protected:
  QScopedPointer<qSlicerFacetedVisualizerModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:


  Q_DECLARE_PRIVATE(qSlicerFacetedVisualizerModuleWidget);
  Q_DISABLE_COPY(qSlicerFacetedVisualizerModuleWidget);

  void UpdateResultsTree(bool visualizedResults);

  void UpdateQueryLogView();

  bool readyNextMRMLDBAtomMatch;

  unsigned int  selectedMRMLAtomIndex;

  std::string mrmlAtom;

  std::string DBAtom;

  //BTX
   std::list< std::string >         queryLog;

   std::vector< std::vector< std::string > > matchingDBAtoms;
   std::vector< std::string > unMatchedMRMLAtoms;

   void GetUserMatchesForMRMLTerms(std::string unmatched);
  //ETX

   bool                                    setDBFile;
   unsigned int                            maxQueryLog;
};

#endif
