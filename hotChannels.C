void hotChannels()
{
  TFile *myFile = TFile::Open("GammaCalo_LHC13g-pass1_60.root");
  TList *myL = (TList*)myFile->Get("GammaCalo;1");
  TList* cut_folder = (TList*)myL->FindObject("Cut Number 00000113_1111121063032220000_0163103100000050");
  TList* esdHists = (TList*)cut_folder->FindObject("00000113_1111121063032220000_0163103100000050 ESD histograms");

}
