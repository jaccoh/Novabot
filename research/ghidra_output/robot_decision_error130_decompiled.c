// robot_decision (mower fw v6.0.2, unchanged in custom builds) — error 130 (0x82, localization quality very bad) handling.
// checkLocalizationStatus: during a task. coverStopDeal: what a stop does. coverContinueDeal: what can be resumed.
// Functions referencing "loc_bad"

// ==================== RobotDecision::checkLocalizationStatus @ 0018d6e0 ====================

/* RobotDecision::checkLocalizationStatus() */

void __thiscall RobotDecision::checkLocalizationStatus(RobotDecision *this)

{
  RobotDecision *pRVar1;
  undefined *puVar2;
  char cVar3;
  int iVar4;
  undefined8 uVar5;
  long lVar6;
  size_t sVar7;
  allocator *paVar8;
  undefined4 local_8b8;
  undefined1 local_8b4;
  long local_8b0;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_8a8;
  code *local_8a0;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_898;
  RobotDecision *local_890;
  SendGoalOptions aSStack_888 [16];
  undefined8 local_878;
  undefined8 local_858;
  undefined8 local_838;
  function<void(rclcpp_action::ClientGoalHandle<decision_msgs::action::LocRecoverMoving>::WrappedResult_const&)>
  afStack_828 [16];
  undefined8 local_818;
  undefined1 auStack_808 [1024];
  undefined1 *local_408 [2];
  undefined1 auStack_3f8 [1008];
  long local_8;
  
  pRVar1 = this + 0x7c9;
  local_8 = *(long *)PTR___stack_chk_guard_00448f98;
  if (((int)(uint)(byte)this[0x636] <= (int)(char)*pRVar1) && (*(int *)(this + 0x388) == 0x82)) {
    updateErrorStatus(this,0,0);
    goto LAB_0018d750;
  }
  if (((byte)this[900] < 0x5a) || ('F' < (char)*pRVar1)) {
    if ((*(int *)(this + 0x7f0) == 5) && (*(int *)(this + 0x380) == 1)) {
LAB_0018d968:
      if ((byte)this[0x4c3] < 5) {
        setPerceptionLevel(this);
      }
    }
  }
  else if (*(int *)(this + 0x380) == 1) {
    if ((this[0x7ac] == (RobotDecision)0x0) && (DAT_0039e890 < *(double *)(this + 0x438))) {
      if (*(int *)(this + 0x7f0) != 5) {
        if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
           (iVar4 = rcutils_logging_initialize(), puVar2 = PTR_stderr_00448e50, iVar4 != 0)) {
          fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:3377] error initializing logging: "
                 ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
          rcutils_get_error_string(auStack_808);
          rcutils_get_error_string(local_408);
          sVar7 = strlen((char *)local_408);
          fwrite(auStack_808,1,sVar7,*(FILE **)puVar2);
          fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar2);
          rcutils_reset_error();
        }
        rclcpp::Node::get_logger();
        uVar5 = 0;
        if (local_8a0 != (code *)0x0) {
          uVar5 = *(undefined8 *)local_8a0;
        }
                    /* try { // try from 0018d8ec to 0018d8ef has its CatchHandler @ 0018dfa0 */
        cVar3 = rcutils_logging_logger_is_enabled_for(uVar5,0x1e);
        if (local_898 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
          std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_898);
        }
        if (cVar3 != '\0') {
          rclcpp::Node::get_logger();
          uVar5 = 0;
          if (local_8a0 != (code *)0x0) {
            uVar5 = *(undefined8 *)local_8a0;
          }
                    /* try { // try from 0018d938 to 0018d93b has its CatchHandler @ 0018df74 */
          rcutils_log(checkLocalizationStatus()::__rcutils_logging_location,0x1e,uVar5,
                      "Test Version: Set high level perception level to let robot cannot out of Lawn!!!!"
                     );
          if (local_898 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
            std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_898);
          }
        }
        setPerceptionLevel(this,5);
      }
      goto LAB_0018d750;
    }
    if (*(int *)(this + 0x7f0) == 5) goto LAB_0018d968;
  }
  if ((((this[0x602] != (RobotDecision)0x0) || ((int)(uint)(byte)this[0x634] <= (int)(char)*pRVar1))
      || ((byte)((char)this[900] - 0x3cU) < 10)) ||
     (((byte)((char)this[0x385] - 0x3cU) < 10 ||
      ((byte)this[0x385] < 0x5a && (byte)this[900] < 0x5a)))) goto LAB_0018d750;
  if (((this[0x630] != (RobotDecision)0x0) &&
      ((cVar3 = rclcpp_action::ClientBase::wait_for_action_server_nanoseconds
                          (*(undefined8 *)(this + 0x14b8),100000000), cVar3 != '\0' &&
       (*(int *)(this + 0x380) != 2)))) && ('\n' < (char)*pRVar1)) {
    if (0x59 < (byte)this[900]) {
      coverStopDeal(this,'=');
    }
    if (0x59 < (byte)this[0x385]) {
      rechargeCancelDeal(this,'=');
    }
    if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
       (iVar4 = rcutils_logging_initialize(), puVar2 = PTR_stderr_00448e50, iVar4 != 0)) {
      fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:3421] error initializing logging: "
             ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
      rcutils_get_error_string(auStack_808);
      rcutils_get_error_string(local_408);
      sVar7 = strlen((char *)local_408);
      fwrite(auStack_808,1,sVar7,*(FILE **)puVar2);
      fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar2);
      rcutils_reset_error();
    }
    rclcpp::Node::get_logger();
    uVar5 = 0;
    if (local_8a0 != (code *)0x0) {
      uVar5 = *(undefined8 *)local_8a0;
    }
                    /* try { // try from 0018dab0 to 0018dab3 has its CatchHandler @ 0018dfa4 */
    cVar3 = rcutils_logging_logger_is_enabled_for(uVar5,0x1e);
    if (local_898 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_898);
    }
    if (cVar3 != '\0') {
      rclcpp::Node::get_logger();
      uVar5 = 0;
      if (local_8a0 != (code *)0x0) {
        uVar5 = *(undefined8 *)local_8a0;
      }
                    /* try { // try from 0018db04 to 0018db07 has its CatchHandler @ 0018df54 */
      rcutils_log(checkLocalizationStatus()::__rcutils_logging_location,0x1e,uVar5,
                  "Current loc confidence: %d",(int)(char)*pRVar1);
      if (local_898 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_898);
      }
    }
    local_8b8 = 0x44960000;
    local_8b4 = 0;
    local_8a0 = locRecoverResultCallback;
    local_898 = (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0;
    local_878 = 0;
    local_858 = 0;
    local_838 = 0;
    local_818 = 0;
    local_890 = this;
                    /* try { // try from 0018db50 to 0018db73 has its CatchHandler @ 0018df4c */
    std::
    function<void(rclcpp_action::ClientGoalHandle<decision_msgs::action::LocRecoverMoving>::WrappedResult_const&)>
    ::operator=(afStack_828,(_Bind *)&local_8a0);
    *(undefined4 *)(this + 0x1588) = 0;
    rclcpp_action::Client<decision_msgs::action::LocRecoverMoving>::async_send_goal
              (*(LocRecoverMoving_Goal_ **)(this + 0x14b8),(SendGoalOptions *)&local_8b8);
                    /* try { // try from 0018db78 to 0018db7b has its CatchHandler @ 0018df34 */
    lVar6 = std::
            __basic_future<std::shared_ptr<rclcpp_action::ClientGoalHandle<decision_msgs::action::LocRecoverMoving>>>
            ::_M_get_result((__basic_future<std::shared_ptr<rclcpp_action::ClientGoalHandle<decision_msgs::action::LocRecoverMoving>>>
                             *)&local_8a0);
    std::
    __shared_ptr<rclcpp_action::ClientGoalHandle<decision_msgs::action::LocRecoverMoving>,(__gnu_cxx::_Lock_policy)2>
    ::__shared_ptr((__shared_ptr<rclcpp_action::ClientGoalHandle<decision_msgs::action::LocRecoverMoving>,(__gnu_cxx::_Lock_policy)2>
                    *)&local_8b0,(__shared_ptr *)(lVar6 + 0x10));
    if (local_898 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_898);
    }
    if (local_8b0 == 0) {
                    /* try { // try from 0018de84 to 0018df03 has its CatchHandler @ 0018df78 */
      if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
         (iVar4 = rcutils_logging_initialize(), iVar4 != 0)) {
        fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:3434] error initializing logging: "
               ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
        rcutils_get_error_string(auStack_808);
        rcutils_get_error_string(local_408);
        sVar7 = strlen((char *)local_408);
        fwrite(auStack_808,1,sVar7,*(FILE **)PTR_stderr_00448e50);
        fwrite(&DAT_0036aa20,1,1,*(FILE **)PTR_stderr_00448e50);
        rcutils_reset_error();
      }
                    /* try { // try from 0018dc98 to 0018dc9b has its CatchHandler @ 0018df78 */
      rclcpp::Node::get_logger();
      uVar5 = 0;
      if (local_8a0 != (code *)0x0) {
        uVar5 = *(undefined8 *)local_8a0;
      }
                    /* try { // try from 0018dcac to 0018dcaf has its CatchHandler @ 0018df08 */
      cVar3 = rcutils_logging_logger_is_enabled_for(uVar5,0x14);
      if (local_898 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_898);
      }
      if (cVar3 != '\0') {
        rclcpp::Node::get_logger();
        uVar5 = 0;
        if (local_8a0 != (code *)0x0) {
          uVar5 = *(undefined8 *)local_8a0;
        }
                    /* try { // try from 0018ddec to 0018ddef has its CatchHandler @ 0018dfa8 */
        rcutils_log(checkLocalizationStatus()::__rcutils_logging_location,0x14,uVar5,
                    "Loc recover action is not ready");
        if (local_898 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
          std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_898);
        }
      }
      if (0x59 < (byte)this[900]) {
                    /* try { // try from 0018dda4 to 0018ddc7 has its CatchHandler @ 0018df78 */
        coverStopDeal(this,'\r');
        updateErrorStatus(this,0x82,0);
      }
      if (0x59 < (byte)this[0x385]) {
                    /* try { // try from 0018dce8 to 0018dcfb has its CatchHandler @ 0018df78 */
        rechargeCancelDeal(this,'\x01');
        updateErrorStatus(this,0x82,0);
      }
    }
    if (local_8a8 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_8a8);
    }
    rclcpp_action::Client<decision_msgs::action::LocRecoverMoving>::SendGoalOptions::
    ~SendGoalOptions(aSStack_888);
    goto LAB_0018d750;
  }
  if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
     (iVar4 = rcutils_logging_initialize(), puVar2 = PTR_stderr_00448e50, iVar4 != 0)) {
    fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:3395] error initializing logging: "
           ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
    rcutils_get_error_string(auStack_808);
    rcutils_get_error_string(local_408);
    sVar7 = strlen((char *)local_408);
    fwrite(auStack_808,1,sVar7,*(FILE **)puVar2);
    fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar2);
    rcutils_reset_error();
  }
  rclcpp::Node::get_logger();
  uVar5 = 0;
  if (local_8a0 != (code *)0x0) {
    uVar5 = *(undefined8 *)local_8a0;
  }
                    /* try { // try from 0018d804 to 0018d807 has its CatchHandler @ 0018dfac */
  cVar3 = rcutils_logging_logger_is_enabled_for(uVar5,0x1e);
  if (local_898 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
    std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_898);
  }
  if (cVar3 != '\0') {
    rclcpp::Node::get_logger();
    uVar5 = 0;
    if (local_8a0 != (code *)0x0) {
      uVar5 = *(undefined8 *)local_8a0;
    }
                    /* try { // try from 0018dbf4 to 0018dbf7 has its CatchHandler @ 0018dfb0 */
    rcutils_log(checkLocalizationStatus()::__rcutils_logging_location,0x1e,uVar5,
                "Current loc confidence: %d",(int)(char)*pRVar1);
    if (local_898 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_898);
    }
  }
  iVar4 = *(int *)(this + 0x380);
  if (iVar4 == 1) {
    if ((byte)this[900] < 0x5a) {
      if ((byte)this[0x385] < 0x5a) goto LAB_0018d750;
    }
    else {
      coverStopDeal(this,'\x0f');
      paVar8 = (allocator *)0x0;
      updateErrorStatus(this,0x82);
      std::__cxx11::string::string<std::allocator<char>>((string *)local_408,"loc_bad",paVar8);
                    /* try { // try from 0018da08 to 0018da0b has its CatchHandler @ 0018df80 */
      saveImage(this,(string *)local_408);
      if (local_408[0] != auStack_3f8) {
        operator_delete(local_408[0]);
      }
      if ((byte)this[0x385] < 0x5a) {
        iVar4 = *(int *)(this + 0x380);
        goto LAB_0018d864;
      }
    }
    rechargeCancelDeal(this,'\x01');
    updateErrorStatus(this,0x82,0);
    iVar4 = *(int *)(this + 0x380);
  }
LAB_0018d864:
  if (((iVar4 == 2) && ((int)(char)*pRVar1 < (int)(uint)(byte)this[0x633])) && (9 < (byte)this[900])
     ) {
    updateErrorStatus(this,0x82,0);
  }
LAB_0018d750:
  if (local_8 - *(long *)PTR___stack_chk_guard_00448f98 == 0) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail(PTR___stack_chk_guard_00448f98,local_8 - *(long *)PTR___stack_chk_guard_00448f98,
                   0);
}


// ==================== RobotDecision::coverStopDeal @ 00185fc0 ====================

/* RobotDecision::coverStopDeal(unsigned char) */

void __thiscall RobotDecision::coverStopDeal(RobotDecision *this,uchar param_1)

{
  undefined *puVar1;
  char cVar2;
  RobotDecision RVar3;
  int iVar4;
  int iVar5;
  undefined8 uVar6;
  size_t sVar7;
  long lVar8;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_840;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_838;
  _State_baseV2 *local_830;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_828;
  undefined8 *local_820;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_818;
  undefined1 auStack_808 [1024];
  char acStack_408 [16];
  code *local_3f8;
  long local_8;
  
  local_8 = *(long *)PTR___stack_chk_guard_00448f98;
  if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
     (iVar4 = rcutils_logging_initialize(0,PTR___stack_chk_guard_00448f98,0),
     puVar1 = PTR_stderr_00448e50, iVar4 != 0)) {
    fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:2147] error initializing logging: "
           ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
    rcutils_get_error_string(auStack_808);
    rcutils_get_error_string(acStack_408);
    sVar7 = strlen(acStack_408);
    fwrite(auStack_808,1,sVar7,*(FILE **)puVar1);
    fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar1);
    rcutils_reset_error();
  }
  rclcpp::Node::get_logger();
  uVar6 = 0;
  if (local_820 != (undefined8 *)0x0) {
    uVar6 = *local_820;
  }
                    /* try { // try from 00186024 to 00186027 has its CatchHandler @ 00186ca8 */
  cVar2 = rcutils_logging_logger_is_enabled_for(uVar6,0x14);
  if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
    std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
  }
  if (cVar2 != '\0') {
    rclcpp::Node::get_logger();
    uVar6 = 0;
    if (local_820 != (undefined8 *)0x0) {
      uVar6 = *local_820;
    }
                    /* try { // try from 001861cc to 001861cf has its CatchHandler @ 00186ca4 */
    rcutils_log(coverStopDeal(unsigned_char)::__rcutils_logging_location,0x14,uVar6,
                "Try to deal with coverage stop request!!!");
    if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
    }
  }
  if (*(int *)(this + 0x380) == 1) {
    RVar3 = this[900];
    if (((byte)RVar3 & 0xf7) == 1) {
      if (0x31 < (byte)this[0x385]) {
        if ((byte)(param_1 - 0x3c) < 10) {
          rechargeCancelDeal(this,param_1);
        }
        else {
          rechargeCancelDeal(this,'\x02');
        }
        goto LAB_00186090;
      }
      if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
         (iVar4 = rcutils_logging_initialize(), puVar1 = PTR_stderr_00448e50, iVar4 != 0)) {
        fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:2175] error initializing logging: "
               ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
        rcutils_get_error_string(auStack_808);
        rcutils_get_error_string(acStack_408);
        sVar7 = strlen(acStack_408);
        fwrite(auStack_808,1,sVar7,*(FILE **)puVar1);
        fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar1);
        rcutils_reset_error();
      }
      rclcpp::Node::get_logger();
      uVar6 = 0;
      if (local_820 != (undefined8 *)0x0) {
        uVar6 = *local_820;
      }
                    /* try { // try from 00186148 to 0018614b has its CatchHandler @ 00186c80 */
      cVar2 = rcutils_logging_logger_is_enabled_for(uVar6,0x28);
      if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
      }
      if (cVar2 == '\0') goto LAB_00186090;
      rclcpp::Node::get_logger();
      uVar6 = 0;
      if (local_820 != (undefined8 *)0x0) {
        uVar6 = *local_820;
      }
                    /* try { // try from 00186194 to 00186197 has its CatchHandler @ 00186bec */
      rcutils_log(coverStopDeal(unsigned_char)::__rcutils_logging_location,0x28,uVar6,&DAT_00371270)
      ;
    }
    else {
      if ((byte)((char)RVar3 - 0x3cU) < 10) {
        if ((byte)(param_1 - 0x3c) < 10) goto LAB_00186090;
        if (RVar3 == (RobotDecision)0x3f) {
          local_3f8 = (code *)0x0;
                    /* try { // try from 00186814 to 00186817 has its CatchHandler @ 00186c1c */
          rclcpp_action::Client<decision_msgs::action::SlipEscaping>::async_cancel_all_goals
                    ((__shared_ptr<std_srvs::srv::SetBool_Request_<std::allocator<void>>,(__gnu_cxx::_Lock_policy)2>
                      *)&local_820,*(undefined8 *)(this + 0x14c8),acStack_408);
          if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
            std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
          }
          if (local_3f8 != (code *)0x0) {
            (*local_3f8)(acStack_408,acStack_408,3);
          }
          RVar3 = this[900];
        }
        if (RVar3 == (RobotDecision)0x3d || RVar3 == (RobotDecision)0x40) {
          local_3f8 = (code *)0x0;
                    /* try { // try from 001866e8 to 001866eb has its CatchHandler @ 00186c58 */
          rclcpp_action::Client<decision_msgs::action::LocRecoverMoving>::async_cancel_all_goals
                    ((__shared_ptr<std_srvs::srv::SetBool_Request_<std::allocator<void>>,(__gnu_cxx::_Lock_policy)2>
                      *)&local_820,*(undefined8 *)(this + 0x14b8),acStack_408);
          if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
            std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
          }
          if (local_3f8 != (code *)0x0) {
            (*local_3f8)(acStack_408,acStack_408,3);
          }
          RVar3 = this[900];
          if (RVar3 != (RobotDecision)0x40) goto LAB_00186218;
          if (this[0x6d1] == (RobotDecision)0x0) {
            loadMap(this);
            RVar3 = this[900];
            goto LAB_00186218;
          }
        }
        else {
LAB_00186218:
          if (RVar3 == (RobotDecision)0x3e) {
            if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
               (iVar4 = rcutils_logging_initialize(), puVar1 = PTR_stderr_00448e50, iVar4 != 0)) {
              fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:2195] error initializing logging: "
                     ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
              rcutils_get_error_string(auStack_808);
              rcutils_get_error_string(acStack_408);
              sVar7 = strlen(acStack_408);
              fwrite(auStack_808,1,sVar7,*(FILE **)puVar1);
              fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar1);
              rcutils_reset_error();
            }
            rclcpp::Node::get_logger();
            uVar6 = 0;
            if (local_820 != (undefined8 *)0x0) {
              uVar6 = *local_820;
            }
                    /* try { // try from 00186248 to 0018624b has its CatchHandler @ 00186bc8 */
            cVar2 = rcutils_logging_logger_is_enabled_for(uVar6,0x28);
            if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
              std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
            }
            if (cVar2 != '\0') {
              rclcpp::Node::get_logger();
              uVar6 = 0;
              if (local_820 != (undefined8 *)0x0) {
                uVar6 = *local_820;
              }
                    /* try { // try from 001867e0 to 001867e3 has its CatchHandler @ 00186c9c */
              rcutils_log(coverStopDeal(unsigned_char)::__rcutils_logging_location,0x28,uVar6,
                          "!!!!!!!!!   Error status, code is not finished !!!!!!!!!");
              if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
                std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
              }
            }
          }
        }
        updateWorkStatus(this,param_1,(uchar)this[0x385]);
        goto LAB_00186090;
      }
      if (0x27 < (byte)((char)RVar3 - 10U)) {
        if ((0x59 < (byte)RVar3) && (this[0x730] == (RobotDecision)0x0)) {
          cVar2 = rclcpp::ClientBase::wait_for_service_nanoseconds
                            (*(undefined8 *)(this + 0xd30),500000000);
          if (cVar2 == '\0') {
            updateWorkStatus(this,'\x01','\0');
            updateErrorStatus(this,0x6e,0);
          }
          else {
            rclcpp::Clock::now();
                    /* try { // try from 00186518 to 0018651b has its CatchHandler @ 00186c44 */
            rclcpp::Time::operator=
                      (this + 0x700,
                       (__shared_ptr<std_srvs::srv::SetBool_Request_<std::allocator<void>>,(__gnu_cxx::_Lock_policy)2>
                        *)&local_820);
            rclcpp::Time::~Time((__shared_ptr<std_srvs::srv::SetBool_Request_<std::allocator<void>>,(__gnu_cxx::_Lock_policy)2>
                                 *)&local_820);
            iVar4 = 3;
            do {
              while( true ) {
                local_840 = (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0;
                local_838 = operator_new(0x18);
                local_840 = local_838 + 0x10;
                *(undefined ***)local_838 = &PTR___Sp_counted_ptr_inplace_00438080;
                *(undefined8 *)(local_838 + 8) = 0x100000001;
                local_838[0x10] = (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>)0x1;
                uVar6 = *(undefined8 *)(this + 0xd30);
                std::
                __shared_ptr<std_srvs::srv::SetBool_Request_<std::allocator<void>>,(__gnu_cxx::_Lock_policy)2>
                ::__shared_ptr((__shared_ptr<std_srvs::srv::SetBool_Request_<std::allocator<void>>,(__gnu_cxx::_Lock_policy)2>
                                *)&local_820,(__shared_ptr *)&local_840);
                    /* try { // try from 00186598 to 0018659b has its CatchHandler @ 00186ba0 */
                rclcpp::Client<std_srvs::srv::SetBool>::async_send_request
                          ((__basic_future<std::shared_ptr<std_srvs::srv::SetBool_Response_<std::allocator<void>>>>
                            *)&local_830,uVar6,
                           (__shared_ptr<std_srvs::srv::SetBool_Request_<std::allocator<void>>,(__gnu_cxx::_Lock_policy)2>
                            *)&local_820);
                if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
                  std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
                }
                local_820 = (undefined8 *)0x3;
                if (local_830 == (_State_baseV2 *)0x0) {
                  uVar6 = std::__throw_future_error(3);
                    /* catch() { ... } // from try @ 00186598 with catch @ 00186ba0 */
                  if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
                    std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
                  }
                  if (local_838 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
                    std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_838);
                  }
                    /* WARNING: Subroutine does not return */
                  _Unwind_Resume(uVar6);
                }
                    /* try { // try from 001865bc to 001865ef has its CatchHandler @ 00186c94 */
                iVar5 = std::__future_base::_State_baseV2::wait_for<long,std::ratio<1l,1l>>
                                  (local_830,(duration *)&local_820);
                if (iVar5 == 0) {
                  lVar8 = std::
                          __basic_future<std::shared_ptr<std_srvs::srv::SetBool_Response_<std::allocator<void>>>>
                          ::_M_get_result((__basic_future<std::shared_ptr<std_srvs::srv::SetBool_Response_<std::allocator<void>>>>
                                           *)&local_830);
                  if (**(char **)(lVar8 + 0x10) != '\0') {
                    this[0x730] = (RobotDecision)0x1;
                    updateWorkStatus(this,param_1,'\0');
                    if (local_828 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
                      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_828);
                    }
                    if (local_838 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
                      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_838);
                    }
                    goto LAB_0018664c;
                  }
                    /* try { // try from 00186ac8 to 00186b9f has its CatchHandler @ 00186c94 */
                  if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
                     (iVar5 = rcutils_logging_initialize(), iVar5 != 0)) {
                    fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:2224] error initializing logging: "
                           ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
                    rcutils_get_error_string(auStack_808);
                    rcutils_get_error_string(acStack_408);
                    sVar7 = strlen(acStack_408);
                    fwrite(auStack_808,1,sVar7,*(FILE **)PTR_stderr_00448e50);
                    fwrite(&DAT_0036aa20,1,1,*(FILE **)PTR_stderr_00448e50);
                    rcutils_reset_error();
                  }
                  rclcpp::Node::get_logger();
                  uVar6 = 0;
                  if (local_820 != (undefined8 *)0x0) {
                    uVar6 = *local_820;
                  }
                    /* try { // try from 00186600 to 00186603 has its CatchHandler @ 00186c8c */
                  cVar2 = rcutils_logging_logger_is_enabled_for(uVar6,0x1e);
                  if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
                    std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
                  }
                  if (cVar2 != '\0') {
                    /* try { // try from 00186870 to 00186873 has its CatchHandler @ 00186c94 */
                    rclcpp::Node::get_logger();
                    uVar6 = 0;
                    if (local_820 != (undefined8 *)0x0) {
                      uVar6 = *local_820;
                    }
                    /* try { // try from 00186888 to 0018688b has its CatchHandler @ 00186bf0 */
                    rcutils_log(coverStopDeal(unsigned_char)::__rcutils_logging_location,0x1e,uVar6,
                                "Stop failed!!!!, Retry again!!!");
                    if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
                      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
                    }
                  }
                }
                local_820 = (undefined8 *)0x1;
                    /* try { // try from 00186628 to 0018662b has its CatchHandler @ 00186c94 */
                std::this_thread::sleep_for<long,std::ratio<1l,1l>>((duration *)&local_820);
                if (local_828 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
                  std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_828);
                }
                if (local_838 == (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) break;
                std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_838);
                iVar4 = iVar4 + -1;
                if (iVar4 == 0) goto LAB_0018664c;
              }
              iVar4 = iVar4 + -1;
            } while (iVar4 != 0);
LAB_0018664c:
            if (this[0x730] == (RobotDecision)0x0) {
              coverCancelDeal(this);
              updateErrorStatus(this,0x6d,0);
              updateWorkStatus(this,'\x01','\0');
            }
          }
        }
        goto LAB_00186090;
      }
      cVar2 = *PTR_g_rcutils_logging_initialized_00448fa8;
      if (RVar3 == (RobotDecision)0x31) {
        if ((cVar2 == '\0') &&
           (iVar4 = rcutils_logging_initialize(), puVar1 = PTR_stderr_00448e50, iVar4 != 0)) {
          fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:2202] error initializing logging: "
                 ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
          rcutils_get_error_string(auStack_808);
          rcutils_get_error_string(acStack_408);
          sVar7 = strlen(acStack_408);
          fwrite(auStack_808,1,sVar7,*(FILE **)puVar1);
          fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar1);
          rcutils_reset_error();
        }
        rclcpp::Node::get_logger();
        uVar6 = 0;
        if (local_820 != (undefined8 *)0x0) {
          uVar6 = *local_820;
        }
                    /* try { // try from 00186480 to 00186483 has its CatchHandler @ 00186ca0 */
        cVar2 = rcutils_logging_logger_is_enabled_for(uVar6,0x28);
        if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
          std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
        }
        if (cVar2 == '\0') goto LAB_00186090;
        rclcpp::Node::get_logger();
        uVar6 = 0;
        if (local_820 != (undefined8 *)0x0) {
          uVar6 = *local_820;
        }
                    /* try { // try from 001864cc to 001864cf has its CatchHandler @ 00186c88 */
        rcutils_log(coverStopDeal(unsigned_char)::__rcutils_logging_location,0x28,uVar6,
                    "Can\'t not start task when in continue status!!!!");
      }
      else {
        if (0x59 < (byte)this[0x385]) {
          if ((cVar2 == '\0') &&
             (iVar4 = rcutils_logging_initialize(), puVar1 = PTR_stderr_00448e50, iVar4 != 0)) {
            fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:2204] error initializing logging: "
                   ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
            rcutils_get_error_string(auStack_808);
            rcutils_get_error_string(acStack_408);
            sVar7 = strlen(acStack_408);
            fwrite(auStack_808,1,sVar7,*(FILE **)puVar1);
            fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar1);
            rcutils_reset_error();
          }
          rclcpp::Node::get_logger();
          uVar6 = 0;
          if (local_820 != (undefined8 *)0x0) {
            uVar6 = *local_820;
          }
                    /* try { // try from 00186348 to 0018634b has its CatchHandler @ 00186c84 */
          cVar2 = rcutils_logging_logger_is_enabled_for(uVar6,0x1e);
          if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
            std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
          }
          if (cVar2 != '\0') {
            rclcpp::Node::get_logger();
            uVar6 = 0;
            if (local_820 != (undefined8 *)0x0) {
              uVar6 = *local_820;
            }
                    /* try { // try from 00186394 to 00186397 has its CatchHandler @ 00186c14 */
            rcutils_log(coverStopDeal(unsigned_char)::__rcutils_logging_location,0x1e,uVar6,
                        "Robot is in recharging status, cancel recharge");
            if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
              std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
            }
          }
          rechargeCancelDeal(this,'\n');
          goto LAB_00186090;
        }
        if ((cVar2 == '\0') &&
           (iVar4 = rcutils_logging_initialize(), puVar1 = PTR_stderr_00448e50, iVar4 != 0)) {
          fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:2207] error initializing logging: "
                 ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
          rcutils_get_error_string(auStack_808);
          rcutils_get_error_string(acStack_408);
          sVar7 = strlen(acStack_408);
          fwrite(auStack_808,1,sVar7,*(FILE **)puVar1);
          fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar1);
          rcutils_reset_error();
        }
        rclcpp::Node::get_logger();
        uVar6 = 0;
        if (local_820 != (undefined8 *)0x0) {
          uVar6 = *local_820;
        }
                    /* try { // try from 00186680 to 00186683 has its CatchHandler @ 00186c18 */
        cVar2 = rcutils_logging_logger_is_enabled_for(uVar6,0x1e);
        if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
          std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
        }
        if (cVar2 == '\0') goto LAB_00186090;
        rclcpp::Node::get_logger();
        uVar6 = 0;
        if (local_820 != (undefined8 *)0x0) {
          uVar6 = *local_820;
        }
                    /* try { // try from 001866cc to 001866cf has its CatchHandler @ 00186be8 */
        rcutils_log(coverStopDeal(unsigned_char)::__rcutils_logging_location,0x1e,uVar6,
                    "No need to stop, already in stop status");
      }
    }
  }
  else {
    if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
       (iVar4 = rcutils_logging_initialize(), puVar1 = PTR_stderr_00448e50, iVar4 != 0)) {
      fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:2149] error initializing logging: "
             ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
      rcutils_get_error_string(auStack_808);
      rcutils_get_error_string(acStack_408);
      sVar7 = strlen(acStack_408);
      fwrite(auStack_808,1,sVar7,*(FILE **)puVar1);
      fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar1);
      rcutils_reset_error();
    }
    rclcpp::Node::get_logger();
    uVar6 = 0;
    if (local_820 != (undefined8 *)0x0) {
      uVar6 = *local_820;
    }
                    /* try { // try from 00186074 to 00186077 has its CatchHandler @ 00186cac */
    cVar2 = rcutils_logging_logger_is_enabled_for(uVar6,0x1e);
    if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
    }
    if (cVar2 == '\0') goto LAB_00186090;
    rclcpp::Node::get_logger();
    uVar6 = 0;
    if (local_820 != (undefined8 *)0x0) {
      uVar6 = *local_820;
    }
                    /* try { // try from 001860f0 to 001860f3 has its CatchHandler @ 00186c90 */
    rcutils_log(coverStopDeal(unsigned_char)::__rcutils_logging_location,0x1e,uVar6,
                "No need to stop coverage task!!!, it\'s not in coverage mode");
  }
  if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
    std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
  }
LAB_00186090:
  if (local_8 - *(long *)PTR___stack_chk_guard_00448f98 == 0) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail(local_8 - *(long *)PTR___stack_chk_guard_00448f98,0);
}



// ==================== RobotDecision::coverContinueDeal @ 00193b68 ====================

/* RobotDecision::coverContinueDeal() */

void __thiscall RobotDecision::coverContinueDeal(RobotDecision *this)

{
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *p_Var1;
  RobotDecision RVar2;
  bool bVar3;
  undefined *puVar4;
  char cVar5;
  int iVar6;
  undefined8 uVar7;
  size_t sVar8;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *p_Var9;
  double dVar10;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_850;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_848;
  __basic_future<std::shared_ptr<std_srvs::srv::SetBool_Response_<std::allocator<void>>>>
  a_Stack_840 [8];
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_838;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_830;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_828;
  undefined8 *local_820;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_818;
  undefined1 auStack_808 [1024];
  char acStack_408 [1024];
  long local_8;
  
  local_8 = *(long *)PTR___stack_chk_guard_00448f98;
  if ((*(int *)(this + 0x380) != 1) || (0x27 < (byte)((char)this[900] - 10U))) {
    if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
       (iVar6 = rcutils_logging_initialize(), puVar4 = PTR_stderr_00448e50, iVar6 != 0)) {
      fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:2245] error initializing logging: "
             ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
      rcutils_get_error_string(auStack_808);
      rcutils_get_error_string(acStack_408);
      sVar8 = strlen(acStack_408);
      fwrite(auStack_808,1,sVar8,*(FILE **)puVar4);
      fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar4);
      rcutils_reset_error();
    }
    rclcpp::Node::get_logger();
    uVar7 = 0;
    if (local_820 != (undefined8 *)0x0) {
      uVar7 = *local_820;
    }
                    /* try { // try from 00193bd0 to 00193bd3 has its CatchHandler @ 00194988 */
    cVar5 = rcutils_logging_logger_is_enabled_for(uVar7,0x1e);
    if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
    }
    if (cVar5 == '\0') goto LAB_00193bec;
    rclcpp::Node::get_logger();
    uVar7 = 0;
    if (local_820 != (undefined8 *)0x0) {
      uVar7 = *local_820;
    }
                    /* try { // try from 00193d04 to 00193d07 has its CatchHandler @ 00194900 */
    rcutils_log(coverContinueDeal()::__rcutils_logging_location,0x1e,uVar7,
                "No need to continue coverage task!!!, it\'s not in coverage mode or  working/stop status"
               );
    goto LAB_00193d08;
  }
  if (this[0x6e6] == (RobotDecision)0x0) {
    if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
       (iVar6 = rcutils_logging_initialize(), puVar4 = PTR_stderr_00448e50, iVar6 != 0)) {
      fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:2259] error initializing logging: "
             ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
      rcutils_get_error_string(auStack_808);
      rcutils_get_error_string(acStack_408);
      sVar8 = strlen(acStack_408);
      fwrite(auStack_808,1,sVar8,*(FILE **)puVar4);
      fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar4);
      rcutils_reset_error();
    }
    rclcpp::Node::get_logger();
    uVar7 = 0;
    if (local_820 != (undefined8 *)0x0) {
      uVar7 = *local_820;
    }
                    /* try { // try from 00193c6c to 00193c6f has its CatchHandler @ 00194978 */
    cVar5 = rcutils_logging_logger_is_enabled_for(uVar7,0x1e);
    if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
    }
    if (cVar5 != '\0') {
      rclcpp::Node::get_logger();
      uVar7 = 0;
      if (local_820 != (undefined8 *)0x0) {
        uVar7 = *local_820;
      }
                    /* try { // try from 00193cb8 to 00193cbb has its CatchHandler @ 00194950 */
      rcutils_log(coverContinueDeal()::__rcutils_logging_location,0x1e,uVar7,
                  "Coverage action is stopped!!!, start new task!!!");
      if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
      }
    }
    coverStartDeal(this);
    goto LAB_00193bec;
  }
  updateWorkStatus(this,'8','\0');
  cVar5 = quitPileDeal(this);
  if (cVar5 == '\0') {
    if (this[0x732] != (RobotDecision)0x0) {
      updateWorkStatus(this,'\n','\0');
      *(undefined4 *)(this + 0x732) = 0;
      goto LAB_00193bec;
    }
    if (this[0x734] != (RobotDecision)0x0) {
      coverCancelDeal(this);
      goto LAB_00193bec;
    }
LAB_00193f58:
    updateWorkStatus(this,'\r','\0');
    goto LAB_00193bec;
  }
  RVar2 = this[900];
  cVar5 = *PTR_g_rcutils_logging_initialized_00448fa8;
  if ((byte)((char)RVar2 - 10U) < 0x28 || RVar2 == (RobotDecision)0x38) {
    if (RVar2 != (RobotDecision)0x31) {
      if ((cVar5 == '\0') &&
         (iVar6 = rcutils_logging_initialize(), puVar4 = PTR_stderr_00448e50, iVar6 != 0)) {
        fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:2282] error initializing logging: "
               ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
        rcutils_get_error_string(auStack_808);
        rcutils_get_error_string(acStack_408);
        sVar8 = strlen(acStack_408);
        fwrite(auStack_808,1,sVar8,*(FILE **)puVar4);
        fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar4);
        rcutils_reset_error();
      }
      rclcpp::Node::get_logger();
      uVar7 = 0;
      if (local_820 != (undefined8 *)0x0) {
        uVar7 = *local_820;
      }
                    /* try { // try from 00193e10 to 00193e13 has its CatchHandler @ 00194910 */
      cVar5 = rcutils_logging_logger_is_enabled_for(uVar7,0x14);
      if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
      }
      if (cVar5 != '\0') {
        rclcpp::Node::get_logger();
        uVar7 = 0;
        if (local_820 != (undefined8 *)0x0) {
          uVar7 = *local_820;
        }
                    /* try { // try from 00193e68 to 00193e6b has its CatchHandler @ 00194914 */
        rcutils_log(coverContinueDeal()::__rcutils_logging_location,0x14,uVar7,
                    "Cover task continue, power: %d%%",this[0x75a]);
        if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
          std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
        }
      }
      updateErrorStatus(this,0,0);
      local_820 = (undefined8 *)&DAT_00000258;
      std::this_thread::sleep_for<long,std::ratio<1l,1000l>>((duration *)&local_820);
      checkChassisStatus(this);
      checkCameraStatus(this);
      checkLocalizationStatus(this);
      checkTrapStatus(this);
      monitorErrorStatus(this);
      if (this[0x602] == (RobotDecision)0x0) {
        sensorInit(this);
      }
      setPerceptionLevel(this);
      if (this[0x7ec] != (RobotDecision)0x0) {
        updateErrorStatus(this,0x7c);
        updateWorkStatus(this,'\r',(uchar)this[0x385]);
        goto LAB_00193bec;
      }
      updateWorkStatus(this,'1','\0');
      if (99 < *(int *)(this + 0x388)) {
        if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
           (iVar6 = rcutils_logging_initialize(), puVar4 = PTR_stderr_00448e50, iVar6 != 0)) {
          fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:2300] error initializing logging: "
                 ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
          rcutils_get_error_string(auStack_808);
          rcutils_get_error_string(acStack_408);
          sVar8 = strlen(acStack_408);
          fwrite(auStack_808,1,sVar8,*(FILE **)puVar4);
          fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar4);
          rcutils_reset_error();
        }
        rclcpp::Node::get_logger();
        uVar7 = 0;
        if (local_820 != (undefined8 *)0x0) {
          uVar7 = *local_820;
        }
                    /* try { // try from 00193f2c to 00193f2f has its CatchHandler @ 001948dc */
        cVar5 = rcutils_logging_logger_is_enabled_for(uVar7,0x1e);
        if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
          std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
        }
        if (cVar5 != '\0') {
          rclcpp::Node::get_logger();
          uVar7 = 0;
          if (local_820 != (undefined8 *)0x0) {
            uVar7 = *local_820;
          }
                    /* try { // try from 00194418 to 0019441b has its CatchHandler @ 00194920 */
          rcutils_log(coverContinueDeal()::__rcutils_logging_location,0x1e,uVar7,
                      "Meeting error can not continue task!!!");
          if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
            std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
          }
        }
        if (*(int *)(this + 0x388) == 0x84) {
          updateWorkStatus(this,'\x0f','\0');
          goto LAB_00193bec;
        }
        goto LAB_00193f58;
      }
      if (this[0x733] != (RobotDecision)0x0) {
        p_Var9 = operator_new(0x18);
        *(undefined ***)p_Var9 = &PTR___Sp_counted_ptr_inplace_00438358;
        *(undefined8 *)(p_Var9 + 8) = 0x100000001;
        puVar4 = PTR___pthread_key_create_00448e10;
        local_830 = p_Var9 + 0x10;
        *(undefined4 *)local_830 = 0x3fc00000;
        uVar7 = *(undefined8 *)(this + 0x1000);
        if (puVar4 == (undefined *)0x0) {
          *(undefined4 *)(p_Var9 + 8) = 2;
        }
        else {
          p_Var1 = p_Var9 + 8;
          do {
            cVar5 = '\x01';
            bVar3 = (bool)ExclusiveMonitorPass(p_Var1,0x10);
            if (bVar3) {
              *(int *)p_Var1 = *(int *)p_Var1 + 1;
              cVar5 = ExclusiveMonitorsStatus();
            }
          } while (cVar5 != '\0');
        }
        local_828 = p_Var9;
                    /* try { // try from 001941ac to 001941af has its CatchHandler @ 001948ac */
        rclcpp::Client<nav2_msgs::srv::ClearCostmapAroundRobot>::async_send_request
                  ((duration *)&local_820,uVar7,&local_830);
        if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
          std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
        }
        if (local_828 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
          std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_828);
        }
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(p_Var9);
      }
      cVar5 = rclcpp::ClientBase::wait_for_service_nanoseconds
                        (*(undefined8 *)(this + 0xd30),500000000);
      if (cVar5 == '\0') {
        updateErrorStatus(this,0x6e,0);
        updateWorkStatus(this,'\x01','2');
        goto LAB_00193bec;
      }
      rclcpp::Clock::now();
                    /* try { // try from 0019421c to 00194227 has its CatchHandler @ 00194860 */
      rclcpp::Time::operator-((duration *)&local_820,this + 0x700);
      dVar10 = (double)rclcpp::Duration::seconds();
      rclcpp::Time::~Time((duration *)&local_820);
      if (dVar10 < 0.5) {
        local_820 = (undefined8 *)&DAT_0000012c;
        std::this_thread::sleep_for<long,std::ratio<1l,1000l>>((duration *)&local_820);
      }
      local_850 = (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0;
      local_848 = operator_new(0x18);
      uVar7 = *(undefined8 *)(this + 0xd30);
      local_850 = local_848 + 0x10;
      *(undefined ***)local_848 = &PTR___Sp_counted_ptr_inplace_00438080;
      *(undefined8 *)(local_848 + 8) = 0x100000001;
      local_848[0x10] = (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>)0x0;
      std::
      __shared_ptr<std_srvs::srv::SetBool_Request_<std::allocator<void>>,(__gnu_cxx::_Lock_policy)2>
      ::__shared_ptr((__shared_ptr<std_srvs::srv::SetBool_Request_<std::allocator<void>>,(__gnu_cxx::_Lock_policy)2>
                      *)&local_820,(__shared_ptr *)&local_850);
                    /* try { // try from 00194298 to 0019429b has its CatchHandler @ 00194894 */
      rclcpp::Client<std_srvs::srv::SetBool>::async_send_request
                (a_Stack_840,uVar7,(duration *)&local_820);
      if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
      }
      local_820 = (undefined8 *)0x3;
                    /* try { // try from 001942b8 to 001942bb has its CatchHandler @ 00194874 */
      iVar6 = std::
              __basic_future<std::shared_ptr<std_srvs::srv::SetBool_Response_<std::allocator<void>>>>
              ::wait_for<long,std::ratio<1l,1l>>(a_Stack_840,(duration *)&local_820);
      if (iVar6 == 0) {
        this[0x730] = (RobotDecision)0x0;
                    /* try { // try from 00194748 to 0019475f has its CatchHandler @ 00194874 */
        updateWorkStatus(this,'\\','\0');
        (**(code **)(**(long **)(this + 0x1270) + 0x20))(*(long **)(this + 0x1270),this + 0x500);
      }
      else {
                    /* try { // try from 001942c8 to 001942cb has its CatchHandler @ 00194970 */
        rclcpp::Clock::now();
                    /* try { // try from 001942d8 to 001942e3 has its CatchHandler @ 00194958 */
        rclcpp::Time::operator-((duration *)&local_820,this + 0x700);
        dVar10 = (double)rclcpp::Duration::seconds();
        if ((10.0 <= dVar10) || (*(double *)(this + 0x420) <= 300.0)) {
          rclcpp::Time::~Time((duration *)&local_820);
        }
        else {
          rclcpp::Time::~Time((duration *)&local_820);
                    /* try { // try from 001947b0 to 0019482f has its CatchHandler @ 00194874 */
          if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
             (iVar6 = rcutils_logging_initialize(), iVar6 != 0)) {
            fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:2327] error initializing logging: "
                   ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
            rcutils_get_error_string(auStack_808);
            rcutils_get_error_string(acStack_408);
            sVar8 = strlen(acStack_408);
            fwrite(auStack_808,1,sVar8,*(FILE **)PTR_stderr_00448e50);
            fwrite(&DAT_0036aa20,1,1,*(FILE **)PTR_stderr_00448e50);
            rcutils_reset_error();
          }
                    /* try { // try from 00194518 to 0019451b has its CatchHandler @ 00194874 */
          rclcpp::Node::get_logger();
          uVar7 = 0;
          if (local_820 != (undefined8 *)0x0) {
            uVar7 = *local_820;
          }
                    /* try { // try from 0019452c to 0019452f has its CatchHandler @ 0019494c */
          cVar5 = rcutils_logging_logger_is_enabled_for(uVar7,0x1e);
          if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
            std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
          }
          if (cVar5 != '\0') {
                    /* try { // try from 0019454c to 0019454f has its CatchHandler @ 00194874 */
            rclcpp::Node::get_logger();
            uVar7 = 0;
            if (local_820 != (undefined8 *)0x0) {
              uVar7 = *local_820;
            }
                    /* try { // try from 00194574 to 00194577 has its CatchHandler @ 00194934 */
            rcutils_log(coverContinueDeal()::__rcutils_logging_location,0x1e,uVar7,
                        "Retry stop again, maybe map size is too bigger");
            if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
              std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
            }
          }
          local_820 = (undefined8 *)0x2;
                    /* try { // try from 00194590 to 0019459b has its CatchHandler @ 00194874 */
          std::this_thread::sleep_for<long,std::ratio<1l,1l>>((duration *)&local_820);
          p_Var9 = operator_new(0x18);
          uVar7 = *(undefined8 *)(this + 0xd30);
          *(undefined ***)p_Var9 = &PTR___Sp_counted_ptr_inplace_00438080;
          p_Var9[0x10] = (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>)0x0;
          *(undefined8 *)(p_Var9 + 8) = 0x100000001;
          std::
          __shared_ptr<std_srvs::srv::SetBool_Request_<std::allocator<void>>,(__gnu_cxx::_Lock_policy)2>
          ::__shared_ptr((__shared_ptr<std_srvs::srv::SetBool_Request_<std::allocator<void>>,(__gnu_cxx::_Lock_policy)2>
                          *)&local_820,(__shared_ptr *)&local_850);
                    /* try { // try from 001945d0 to 001945d3 has its CatchHandler @ 00194924 */
          rclcpp::Client<std_srvs::srv::SetBool>::async_send_request
                    ((__basic_future<std::shared_ptr<std_srvs::srv::SetBool_Response_<std::allocator<void>>>>
                      *)&local_830,uVar7,(duration *)&local_820);
          if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
            std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
          }
          local_820 = (undefined8 *)0x3;
                    /* try { // try from 001945f0 to 001945f3 has its CatchHandler @ 001948e0 */
          iVar6 = std::
                  __basic_future<std::shared_ptr<std_srvs::srv::SetBool_Response_<std::allocator<void>>>>
                  ::wait_for<long,std::ratio<1l,1l>>
                            ((__basic_future<std::shared_ptr<std_srvs::srv::SetBool_Response_<std::allocator<void>>>>
                              *)&local_830,(duration *)&local_820);
          if (iVar6 == 0) {
            this[0x730] = (RobotDecision)0x0;
                    /* try { // try from 00194780 to 00194797 has its CatchHandler @ 001948e0 */
            updateWorkStatus(this,'\\','\0');
            (**(code **)(**(long **)(this + 0x1270) + 0x20))(*(long **)(this + 0x1270),this + 0x500)
            ;
            if (local_828 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
              std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_828);
            }
            std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(p_Var9);
            goto LAB_00194320;
          }
          if (local_828 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
            std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_828);
          }
          std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(p_Var9);
        }
                    /* try { // try from 001942fc to 0019431f has its CatchHandler @ 00194874 */
        coverCancelDeal(this);
        updateErrorStatus(this,0x6d,0);
        updateWorkStatus(this,'\x01','\0');
      }
LAB_00194320:
      if (local_838 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_838);
      }
      if (local_848 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_848);
      }
      goto LAB_00193bec;
    }
    if ((cVar5 == '\0') &&
       (iVar6 = rcutils_logging_initialize(), puVar4 = PTR_stderr_00448e50, iVar6 != 0)) {
      fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:2279] error initializing logging: "
             ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
      rcutils_get_error_string(auStack_808);
      rcutils_get_error_string(acStack_408);
      sVar8 = strlen(acStack_408);
      fwrite(auStack_808,1,sVar8,*(FILE **)puVar4);
      fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar4);
      rcutils_reset_error();
    }
    rclcpp::Node::get_logger();
    uVar7 = 0;
    if (local_820 != (undefined8 *)0x0) {
      uVar7 = *local_820;
    }
                    /* try { // try from 001940b8 to 001940bb has its CatchHandler @ 00194908 */
    cVar5 = rcutils_logging_logger_is_enabled_for(uVar7,0x1e);
    if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
    }
    if (cVar5 == '\0') goto LAB_00193bec;
    rclcpp::Node::get_logger();
    uVar7 = 0;
    if (local_820 != (undefined8 *)0x0) {
      uVar7 = *local_820;
    }
                    /* try { // try from 00194104 to 00194107 has its CatchHandler @ 00194980 */
    rcutils_log(coverContinueDeal()::__rcutils_logging_location,0x1e,uVar7,
                "Can\'t not start task when in continue status!!!!");
  }
  else {
    if ((cVar5 == '\0') &&
       (iVar6 = rcutils_logging_initialize(), puVar4 = PTR_stderr_00448e50, iVar6 != 0)) {
      fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:2351] error initializing logging: "
             ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
      rcutils_get_error_string(auStack_808);
      rcutils_get_error_string(acStack_408);
      sVar8 = strlen(acStack_408);
      fwrite(auStack_808,1,sVar8,*(FILE **)puVar4);
      fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar4);
      rcutils_reset_error();
    }
    rclcpp::Node::get_logger();
    uVar7 = 0;
    if (local_820 != (undefined8 *)0x0) {
      uVar7 = *local_820;
    }
                    /* try { // try from 00193f90 to 00193f93 has its CatchHandler @ 00194918 */
    cVar5 = rcutils_logging_logger_is_enabled_for(uVar7,0x1e);
    if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
    }
    if (cVar5 == '\0') goto LAB_00193bec;
    rclcpp::Node::get_logger();
    uVar7 = 0;
    if (local_820 != (undefined8 *)0x0) {
      uVar7 = *local_820;
    }
                    /* try { // try from 00193fdc to 00193fdf has its CatchHandler @ 00194834 */
    rcutils_log(coverContinueDeal()::__rcutils_logging_location,0x1e,uVar7,
                "No need to continue task, when not in stop status");
  }
LAB_00193d08:
  if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
    std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
  }
LAB_00193bec:
  if (local_8 - *(long *)PTR___stack_chk_guard_00448f98 == 0) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail(local_8 - *(long *)PTR___stack_chk_guard_00448f98,0);
}



// ==================== std::_Function_handler<std::unique_ptr<std::__future_base::_Result_base,std::__future_base::_Result_base::_Deleter>(),std::__future_base::_State_baseV2::_Setter<std::shared_ptr<rclcpp_action::ClientGoalHandle<decision_msgs::action::LocRecoverMoving>>,std::shared_ptr<rclcpp_action::ClientGoalHandle<decision_msgs::action::LocRecoverMoving>>&&>>::_M_invoke @ 00197df0 ====================
