// Fill out your copyright notice in the Description page of Project Settings.

#include "StreamingActor.h"
#include "VIMRUE5.h"

void AStreamingActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
  delete vox_merge;
}

void AStreamingActor::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
  if(SendDevicePosesOnTick) SendDevicePoses();
  const auto time_since_last_frame = ms_now - frame_update_timestamp;
  if(time_since_last_frame > 500 && tmp_render_buffers->current_head()->GetData()[0].VoxelCount > 0)
    {
      for( auto& rb : *tmp_render_buffers->current_head()){
        rb.VoxelCount = 0;
    }
    tmp_render_buffers->try_advance_head();
  }
}

bool AStreamingActor::InitComponent(FString _VNetIDSuffix)
{
  InitVimrComponent(_VNetIDSuffix);
  // can't be reinitialising stuff
  if(vox_merge){
    UE_LOG(VIMRLog, Log, TEXT("vox_merge has already been initialised for vnet_suffix: %s"), *_VNetIDSuffix);
    return false;
  }
  AHUD* extantHUD = nullptr;
  auto * world = GetWorld();
  APlayerController * firstPC = nullptr;
  if(world) firstPC = GEngine->GetFirstLocalPlayerController(GetWorld());
  if(firstPC) extantHUD = firstPC->GetHUD();

  if(extantHUD && extantHUD->IsA(AVIMRHUD::StaticClass()))
    hud = static_cast<AVIMRHUD*>(extantHUD);
  UE_LOG(VIMRLog, Log, TEXT("StreamingActor cid: %s"), *ComponentID);
  UE_LOG(VIMRLog, Log, TEXT("StreamingActor icf: %s"), *InstanceConfigFile);
  UE_LOG(VIMRLog, Log, TEXT("StreamingActor lgf: %s"), *LogFolder);
  vox_merge = new VIMR::VoxMergeUESafe();
  if (!vox_merge->init_safe(
    ComponentID.IsEmpty() ? nullptr : TCHAR_TO_ANSI(*ComponentID),
    InstanceConfigFile.IsEmpty() ? nullptr : TCHAR_TO_ANSI(*InstanceConfigFile),
    LogFolder.IsEmpty() ? nullptr : TCHAR_TO_ANSI(*LogFolder)))
  {
    //FIXME: delete vox_merge;
    UE_LOG(VIMRLog, Error, TEXT("Failed to init voxel merge component"));
    SetHUDText(TEXT("Failed to init voxel merge component"));
    delete vox_merge;
    vox_merge = nullptr;
    return false;
  }
  char* inst_id;
  auto inst_id_len = vox_merge->get_instance_id(&inst_id);
  InstanceID = FString(ANSI_TO_TCHAR(inst_id));
  vox_merge->set_vox_sink([this](VIMR::VoxelMessage* _v)
  {
    SetHUDText(FString::Printf(TEXT("SRC: %s\nframe:   %lld\nvox size: %.0fmm\nnum vox: %i\nFPS:    %.2f"), 
      *VNetID, 
      _v->frame_number,
      _v->encoding.get_vox_mm(),
      _v->octree.vox_count(),
      voxUpdateFPS.hz()));
    this->CopyVoxelsToRenderBuffer(*_v);
  });
  SetHUDText(FString::Printf(TEXT("SRC: %s\nframe:   %lld\nvox size: %.0fmm\nnum vox: %i\nFPS:    %.2f"), 
      *VNetID,       0,      0,      0,      0));
  return true;
}

bool AStreamingActor::InitPeerStream(FString _peerInstanceID)
{
  bool isbad = false;
  if(!vox_merge){
    UE_LOG(VIMRLog, Error, TEXT("StreamingActor: VoxMerge not initialized"));
    isbad= true;
  }
  if(ComponentID.IsEmpty())
  {
    UE_LOG(VIMRLog, Error, TEXT("StreamingActor: ComponentID is empty"));
    isbad = true;
  }
  if(InstanceConfigFile.IsEmpty())
  {
    UE_LOG(VIMRLog, Error, TEXT("StreamingActor: InstanceConfigFile is empty"));
    isbad = true;
  }
  if(_peerInstanceID.IsEmpty())
  {
    UE_LOG(VIMRLog, Error, TEXT("StreamingActor: PeerInstanceID is empty"));
    isbad = true;
  }
  if(isbad) return false;

  UE_LOG(VIMRLog, Warning, TEXT("StreamingActor: %s pairing with %s"), *VNetID, *_peerInstanceID)

  return vox_merge->init_remote_stream(TCHAR_TO_ANSI(*_peerInstanceID),TCHAR_TO_ANSI(*VNetID), TCHAR_TO_ANSI(*VNetAddr), VNetIsLAN);
}

void AStreamingActor::BeginPlay()
{
  Super::BeginPlay();
}

void AStreamingActor::SendDevicePoses()
{
  if(!vox_merge) return;
  if(!NewPoseToSend) return;
  VIMR::Pose poses[128];
  int pc = 0;
  for(auto & vrp : VRDevicePoses)
  {
    const auto t = 0.01 * vrp.Value.GetTranslation();
    const auto r = vrp.Value.GetRotation();
    const auto k = static_cast<VIMR::PoseType>(vrp.Key);
    poses[pc] = VIMR::Pose(ms_now, k, t.X, t.Y, t.Z, r.W, r.X, r.Y, r.Z);
    pc++;
  }
  if(!vox_merge->send_poses(poses, pc))
    UE_LOG(VIMRLog, Warning, TEXT("Failed to send poses"))
  NewPoseToSend = false;
}

void AStreamingActor::SetDevicePose(int _idx, FTransform _tx)
{
  VRDevicePoses.Add(_idx, _tx);
}
