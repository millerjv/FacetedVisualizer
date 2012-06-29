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

// Qt includes
#include <QtPlugin>

// FacetedVisualizer Logic includes
#include <vtkSlicerFacetedVisualizerLogic.h>

// FacetedVisualizer includes
#include "qSlicerFacetedVisualizerModule.h"
#include "qSlicerFacetedVisualizerModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerFacetedVisualizerModule, qSlicerFacetedVisualizerModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_FacetedVisualizer
class qSlicerFacetedVisualizerModulePrivate
{
public:
  qSlicerFacetedVisualizerModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerFacetedVisualizerModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerFacetedVisualizerModulePrivate::qSlicerFacetedVisualizerModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerFacetedVisualizerModule methods

//-----------------------------------------------------------------------------
qSlicerFacetedVisualizerModule::qSlicerFacetedVisualizerModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerFacetedVisualizerModulePrivate)
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerFacetedVisualizerModule::categories()const
{
  return QStringList() << "Developer Tools";
}

//-----------------------------------------------------------------------------
qSlicerFacetedVisualizerModule::~qSlicerFacetedVisualizerModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerFacetedVisualizerModule::helpText()const
{
  QString help =
  "This module allows users to visualize 3D atlas and 3D models in a scene by"
    "combining the atlas and the models with an ontology. The module enables "
    "flexible visualization by using faceted search on the ontology. Although, "
    "this module can be used without an ontology to just visualize the user created models."
  "To use this module, first load a mrml Scene, and add all the 3D models. Then "
  "load a .sqlite3 database file that contains the ontology. A query string can "
  "then be used to produce the visualizations. The module supports simple queries"
  "(ex. 'putamen', 'cingulate gyrus'), complex queries (e.g. 'liver + kidney', or 'liver, kidney')"
  ", and specialized queries (e.g. 'liver;arterial supply'). The specialized queries essentially"
    "involve 'AND' on the query.";

  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerFacetedVisualizerModule::acknowledgementText()const
{
  return "This work is part of the Neuroimage Analysis Center (NAC), an NIBIB National Resource Center, Grant P41 EB015902";
}

//-----------------------------------------------------------------------------
QStringList qSlicerFacetedVisualizerModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Harini Veeraraghavan (GE)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerFacetedVisualizerModule::icon()const
{
  return QIcon(":/Icons/FacetedVisualizer.png");
}

//-----------------------------------------------------------------------------
void qSlicerFacetedVisualizerModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerFacetedVisualizerModule::createWidgetRepresentation()
{
  return new qSlicerFacetedVisualizerModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerFacetedVisualizerModule::createLogic()
{
  return vtkSlicerFacetedVisualizerLogic::New();
}
