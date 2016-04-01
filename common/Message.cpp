/*
 * Message.cpp
 *
 *  Created on: May 12, 2013
 *      Author: wangli
 */

#include "Message.h"
using claims::SegmentExecStatus;
void PhysicalQueryPlan::run() {
  SegmentExecStatus* segment_exec_status = new SegmentExecStatus(
      make_pair(query_id_, segment_id_ * kMaxNodeNum + target_node_id_),
      coor_addr_);
  segment_exec_status->RegisterToTracker();

  block_stream_iterator_root_->Open();
  segment_exec_status->UpdateStatus(SegmentExecStatus::ExecStatus::kOk,
                                    "physical plan open() succeed", 0, true);
  while (block_stream_iterator_root_->Next(0)) {
  }
  segment_exec_status->UpdateStatus(SegmentExecStatus::ExecStatus::kOk,
                                    "physical plan next() succeed", 0, true);
  block_stream_iterator_root_->Close();
  segment_exec_status->UpdateStatus(SegmentExecStatus::ExecStatus::kDone,
                                    "physical plan close() succeed", 0, true);
  segment_exec_status->UnRegisterFromTracker();
  delete segment_exec_status;
}
