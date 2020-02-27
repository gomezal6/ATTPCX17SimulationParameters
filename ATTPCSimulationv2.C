#include "TSystem.h"
#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoPgon.h"
#include "TGeoMatrix.h"
#include "TGeoCompositeShape.h"
#include "TFile.h"
#include "TString.h"
#include "TList.h"
#include "TROOT.h"

#include <iostream>

//in root, all sizes are given in cm

// Name of geometry version and output file
const TString geoVersion = "ATTPCSimulationv2.1";
const TString FileName = geoVersion + ".root";
const TString FileName1 = geoVersion + "_geomanager.root";

// Names of the different used materials which are used to build the modules
// The materials are defined in the global media.geo file
const TString MediumGas     = "heco2";
const TString CylinderVolumeMedium         = "steel";
const TString MediumVacuum = "vacuum4";
const TString VacuumTubeWallMedium = "G10";
const TString VacuumTubeCap = "Aluminum";
const TString Target = "LithiumOxide";


// Silicon box for both module types
//ATTPC Parameters
const Float_t First_Z_Position = 10; // Distance of the center of the first detector layer [cm];
const Float_t Z_Distance = 10;       // Distance of the center of the first detector layer [cm];
const Float_t tpc_diameter = 50.;
const Float_t drift_length = 120.;

//G10 tube parameters
const Float_t First_Tube_Zposition = 10;
const Float_t Tube_Z_Distance= 10;
const Float_t tube_OD = 2.74;
const Float_t tube_ID = 2.54;
const Float_t tube_length = 25.4;

//Aluminum pipe parameters
const Float_t First_Altube_Zposition = 10;
const Float_t Altube_Z_Distance= 10;
const Float_t Altube_OD = 2.54;
const Float_t Altube_ID = 2.44;
const Float_t Altube_length = 0.1;

// some global variables
TGeoManager* gGeoMan = new TGeoManager("ATTPC","ATTPC");;  // Pointer to TGeoManager instance
TGeoVolume* gModules; // Global storage for module types

// Forward declarations
void create_materials_from_media_file();
TGeoVolume* create_detector();
TGeoVolume* create_tube();
TGeoVolume* create_Altube();
TGeoVolume* create_G10Vacuum();
TGeoVolume* create_LithiumEvp();
TGeoVolume* create_AlVacuum();
TGeoVolume* create_AlTubeCap();
void position_detector();
void add_alignable_volumes();

void ATTPCSimulationv2() {
  // Load the necessary FairRoot libraries
 // gROOT->LoadMacro("$VMCWORKDIR/gconfig/basiclibs.C");
 // basiclibs();
 // gSystem->Load("libGeoBase");
 // gSystem->Load("libParBase");
 // gSystem->Load("libBase");

  // Load needed material definition from media.geo file
  create_materials_from_media_file();

  // Get the GeoManager for later usage
  gGeoMan = (TGeoManager*) gROOT->FindObject("FAIRGeom");
  gGeoMan->SetVisLevel(7);

  // Create the top volume

  TGeoVolume* top = new TGeoVolumeAssembly("TOP");
  gGeoMan->SetTopVolume(top);

  TGeoMedium* gas   = gGeoMan->GetMedium(MediumVacuum);
  TGeoVolume* tpcvac = new TGeoVolumeAssembly(geoVersion);
  tpcvac -> SetMedium(gas);
  top->AddNode(tpcvac, 1);

  gModules = create_detector();
  gModules = create_tube();
  gModules = create_Altube();
  gModules = create_G10Vacuum();
  gModules = create_LithiumEvp();
  gModules = create_AlVacuum();
  gModules = create_AlTubeCap();

  //position_detector();

  cout<<"Voxelizing."<<endl;
  top->Voxelize("");
  gGeoMan->CloseGeometry();

  //add_alignable_volumes();

  gGeoMan->CheckOverlaps(0.001);
  gGeoMan->PrintOverlaps();
  gGeoMan->Test();

  TFile* outfile = new TFile(FileName,"RECREATE");
  top->Write();
  outfile->Close();

  TFile* outfile1 = new TFile(FileName1,"RECREATE");
  gGeoMan->Write();
  outfile1->Close();

    top->Draw("ogl");
  //top->Raytrace();

}

void create_materials_from_media_file()
{
  // Use the FairRoot geometry interface to load the media which are already defined
  FairGeoLoader* geoLoad = new FairGeoLoader("TGeo", "FairGeoLoader");
  FairGeoInterface* geoFace = geoLoad->getGeoInterface();
  TString geoPath = gSystem->Getenv("VMCWORKDIR");
  TString geoFile = geoPath + "/geometry/media.geo";
  geoFace->setMediaFile(geoFile);
  geoFace->readMedia();

  // Read the required media and create them in the GeoManager
  FairGeoMedia* geoMedia = geoFace->getMedia();
  FairGeoBuilder* geoBuild = geoLoad->getGeoBuilder();

  FairGeoMedium* isobutan              = geoMedia->getMedium("isobutan");
  FairGeoMedium* steel          = geoMedia->getMedium("steel");
  FairGeoMedium* heco2          = geoMedia->getMedium("heco2");
  FairGeoMedium* vacuum4          = geoMedia->getMedium("vacuum4");
  FairGeoMedium* G10          = geoMedia->getMedium("G10");
  FairGeoMedium* Aluminum          = geoMedia->getMedium("Aluminum");
  FairGeoMedium* LithiumOxide          = geoMedia->getMedium("LithiumOxide");

  // include check if all media are found

  geoBuild->createMedium(isobutan);
  geoBuild->createMedium(steel);
   geoBuild->createMedium(heco2);
   geoBuild->createMedium(vacuum4);
   geoBuild->createMedium(G10);
   geoBuild->createMedium(Aluminum);
   geoBuild->createMedium(LithiumOxide);
}

TGeoVolume* create_detector()
{

  // needed materials
  TGeoMedium* OuterCylinder   = gGeoMan->GetMedium(CylinderVolumeMedium);
  TGeoMedium* gas   = gGeoMan->GetMedium(MediumGas);

  TGeoVolume *drift_volume = gGeoManager->MakeTube("drift_volume", gas,0, tpc_diameter/2, drift_length/2);
  //TGeoVolume *drift_volume = gGeoManager->MakeBox("drift_volume", gas,  100./2, 100./2, 100./2);
  gGeoMan->GetVolume(geoVersion)->AddNode(drift_volume,1, new TGeoTranslation(0,0,drift_length/2));
  drift_volume->SetTransparency(80);

  // Single detector_layer
 /* TGeoBBox* det_plane = new TGeoBBox("", Module_Size_X/2., Module_Size_Y/2., Module_Size_Z/2.);
  TGeoVolume* det_plane_vol =
    new TGeoVolume("tut4_det", det_plane, SiliconVolMed);
  det_plane_vol->SetLineColor(kBlue); // set line color
  det_plane_vol->SetTransparency(70); // set transparency
  TGeoTranslation* det_plane_trans
    = new TGeoTranslation("", 0., 0., 0.);

  return det_plane_vol;*/

  return drift_volume;

}

TGeoVolume* create_tube() //Creating G10 tube
{

  // needed materials
  TGeoMedium* OuterTube   = gGeoMan->GetMedium(VacuumTubeWallMedium);
  TGeoMedium* TubeVacuum   = gGeoMan->GetMedium(MediumVacuum);

  TGeoVolume *G10_volume = gGeoManager->MakeTube("G10_volume", OuterTube,tube_ID/2, tube_OD/2, tube_length/2); //third parameter is the ID
  //TGeoVolume *G10_volume = gGeoManager->MakeBox("G10_volume", gas,  100./2, 100./2, 100./2);
  gGeoMan->GetVolume(geoVersion)->AddNode(G10_volume,1, new TGeoTranslation(0,0,tube_length/2));
  G10_volume->SetTransparency(50);
  G10_volume->SetLineColor(kGreen);

  return G10_volume;

}

TGeoVolume* create_Altube() //creating aluminum insert for the G10 tube
{

  // needed materials
  TGeoMedium* OuterTube   = gGeoMan->GetMedium(VacuumTubeCap);
  TGeoMedium* TubeVacuum   = gGeoMan->GetMedium(MediumVacuum);

  TGeoVolume *Altube_volume = gGeoManager->MakeTube("Altube_volume", OuterTube,Altube_ID/2, Altube_OD/2, Altube_length/2); //third parameter is the ID
  //TGeoVolume *Altube_volume = gGeoManager->MakeBox("Altube_volume", gas,  100./2, 100./2, 100./2);
  gGeoMan->GetVolume(geoVersion)->AddNode(Altube_volume,1, new TGeoTranslation(0,0,Altube_length/2+25.4-.024-Altube_length));	//center the aluminum tube (ring in this case) against the back wall and shift it along z axis down the length of G10 pipe, make sure to bring it in a bit for the alum cap
  Altube_volume->SetTransparency(50);
  Altube_volume->SetLineColor(kGray);

  return Altube_volume;

}

TGeoVolume* create_AlTubeCap()	//creating end cap for pipe
{

  // needed materials
  TGeoMedium* TubeCap   = gGeoMan->GetMedium(VacuumTubeCap);

  TGeoVolume *Cap_volume = gGeoManager->MakeTube("Cap_volume", TubeCap,0, Altube_OD/2, 0.024/2); //third parameter is the ID
  //TGeoVolume *Cap_volume = gGeoManager->MakeBox("Cap_volume", gas,  100./2, 100./2, 100./2);
  gGeoMan->GetVolume(geoVersion)->AddNode(Cap_volume,1, new TGeoTranslation(0,0,0.024/2+tube_length-.024));	//center the cap to datum and shift it forward the same distance as length of tube, let it sit on the aluminum ring
  Cap_volume->SetTransparency(50);
  Cap_volume->SetLineColor(kGray);

  return Cap_volume;

}

TGeoVolume* create_G10Vacuum()	//creating a vacuum to be "inserted" into the G10 pipe
{

  // needed materials
  TGeoMedium* TubeVacuum   = gGeoMan->GetMedium(MediumVacuum);

  TGeoVolume *vacuum_volume = gGeoManager->MakeTube("G10vacuum_volume", TubeVacuum,0, tube_ID/2, 25.276/2); //third parameter is the ID
  //TGeoVolume *vacuum_volume = gGeoManager->MakeBox("vacuum_volume", gas,  100./2, 100./2, 100./2);
  gGeoMan->GetVolume(geoVersion)->AddNode(vacuum_volume,1, new TGeoTranslation(0,0,25.276/2));	//USE THIS TO TRANSLATE AND MOVE TUBES
  vacuum_volume->SetTransparency(100);

  return vacuum_volume;

}

TGeoVolume* create_LithiumEvp()	//creating lithium target
{

  // needed materials
  TGeoMedium* LiTarget   = gGeoMan->GetMedium(Target);

  TGeoVolume *Li_volume = gGeoManager->MakeTube("Li_volume", LiTarget,0, Altube_ID/2, 0.000498/2); //third parameter is the ID
  //TGeoVolume *Li_volume = gGeoManager->MakeBox("Li_volume", gas,  100./2, 100./2, 100./2);
  gGeoMan->GetVolume(geoVersion)->AddNode(Li_volume,1, new TGeoTranslation(0,0,0.000498/2+tube_length-.024-0.000498));	//center the cap to datum and shift it forward the same distance as G10 pipe, but bring it in to rest it on inside of cap
  Li_volume->SetTransparency(10);
  Li_volume->SetLineColor(kRed);

  return Li_volume;

}

TGeoVolume* create_AlVacuum()	//creating a vacuum to be "inserted" into the Aluminum pipe
{

  // needed materials
  TGeoMedium* TubeVacuum   = gGeoMan->GetMedium(MediumVacuum);

  TGeoVolume *Alvacuum_volume = gGeoManager->MakeTube("Alvacuum_volume", TubeVacuum,0, Altube_ID/2, Altube_length/2); //third parameter is the ID
  //TGeoVolume *Alvacuum_volume = gGeoManager->MakeBox("Alvacuum_volume", gas,  100./2, 100./2, 100./2);
  gGeoMan->GetVolume(geoVersion)->AddNode(Alvacuum_volume,1, new TGeoTranslation(0,0,Altube_length/2+25.276));	//USE THIS TO TRANSLATE AND MOVE TUBES
  Alvacuum_volume->SetTransparency(100);

  return Alvacuum_volume;

}

void position_detector()
{

  /*TGeoTranslation* det_trans=NULL;

  Int_t numDets=0;
  for (Int_t detectorPlanes = 0; detectorPlanes < 40; detectorPlanes++) {
    det_trans
      = new TGeoTranslation("", 0., 0., First_Z_Position+(numDets*Z_Distance));
    gGeoMan->GetVolume(geoVersion)->AddNode(gModules, numDets, det_trans);
    numDets++;

  }*/


}

void add_alignable_volumes()
{

 /* TString volPath;
  TString symName;
  TString detStr   = "Tutorial4/det";
  TString volStr   = "/TOP_1/tutorial4_1/tut4_det_";

  for (Int_t detectorPlanes = 0; detectorPlanes < 40; detectorPlanes++) {

    volPath  = volStr;
    volPath += detectorPlanes;

    symName  = detStr;
    symName += Form("%02d",detectorPlanes);

    cout<<"Path: "<<volPath<<", "<<symName<<endl;
//    gGeoMan->cd(volPath);

    gGeoMan->SetAlignableEntry(symName.Data(),volPath.Data());

  }
    cout<<"Nr of alignable objects: "<<gGeoMan->GetNAlignable()<<endl;*/

}
