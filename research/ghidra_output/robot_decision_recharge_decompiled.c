// Ghidra decompile of robot_decision (mower fw v6.0.2): recharge + error-status handlers

// ==================== RobotDecision::handleAutoRechargeTask @ 00168158 ====================

/* RobotDecision::handleAutoRechargeTask(std::shared_ptr<rmw_request_id_t>,
   std::shared_ptr<std_srvs::srv::Trigger_Request_<std::allocator<void> > >,
   std::shared_ptr<std_srvs::srv::Trigger_Response_<std::allocator<void> > >) */

void __thiscall
RobotDecision::handleAutoRechargeTask
          (RobotDecision *this,undefined8 param_2,undefined8 param_3,undefined8 *param_4)

{
  undefined *puVar1;
  char cVar2;
  int iVar3;
  undefined8 uVar4;
  size_t sVar5;
  undefined8 *local_818;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_810;
  undefined1 auStack_808 [1024];
  char acStack_408 [1024];
  long local_8;
  
  local_8 = *(long *)PTR___stack_chk_guard_00448f98;
  if (this[0x736] == (RobotDecision)0x0) {
    if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
       (iVar3 = rcutils_logging_initialize(0,0), puVar1 = PTR_stderr_00448e50, iVar3 != 0)) {
      fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:941] error initializing logging: "
             ,1,0x6e,*(FILE **)PTR_stderr_00448e50);
      rcutils_get_error_string(auStack_808);
      rcutils_get_error_string(acStack_408);
      sVar5 = strlen(acStack_408);
      fwrite(auStack_808,1,sVar5,*(FILE **)puVar1);
      fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar1);
      rcutils_reset_error();
    }
    rclcpp::Node::get_logger();
    uVar4 = 0;
    if (local_818 != (undefined8 *)0x0) {
      uVar4 = *local_818;
    }
                    /* try { // try from 00168244 to 00168247 has its CatchHandler @ 00168404 */
    cVar2 = rcutils_logging_logger_is_enabled_for(uVar4,0x14);
    if (local_810 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_810);
    }
    if (cVar2 != '\0') {
      rclcpp::Node::get_logger();
      uVar4 = 0;
      if (local_818 != (undefined8 *)0x0) {
        uVar4 = *local_818;
      }
                    /* try { // try from 00168290 to 00168293 has its CatchHandler @ 0016842c */
      rcutils_log(handleAutoRechargeTask(std::shared_ptr<rmw_request_id_t>,std::shared_ptr<std_srvs::srv::Trigger_Request_<std::allocator<void>>>,std::shared_ptr<std_srvs::srv::Trigger_Response_<std::allocator<void>>>)
                  ::__rcutils_logging_location,0x14,uVar4,
                  "Receiving recharge task no guide pose mode command for mapping task!!!");
      if (local_810 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_810);
      }
    }
    *(undefined4 *)(this + 0x736) = 1;
    *(undefined1 *)*param_4 = 1;
  }
  else {
    if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
       (iVar3 = rcutils_logging_initialize(0,0), puVar1 = PTR_stderr_00448e50, iVar3 != 0)) {
      fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:938] error initializing logging: "
             ,1,0x6e,*(FILE **)PTR_stderr_00448e50);
      rcutils_get_error_string(auStack_808);
      rcutils_get_error_string(acStack_408);
      sVar5 = strlen(acStack_408);
      fwrite(auStack_808,1,sVar5,*(FILE **)puVar1);
      fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar1);
      rcutils_reset_error();
    }
    rclcpp::Node::get_logger();
    uVar4 = 0;
    if (local_818 != (undefined8 *)0x0) {
      uVar4 = *local_818;
    }
                    /* try { // try from 001681cc to 001681cf has its CatchHandler @ 00168428 */
    cVar2 = rcutils_logging_logger_is_enabled_for(uVar4,0x1e);
    if (local_810 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_810);
    }
    if (cVar2 != '\0') {
      rclcpp::Node::get_logger();
      uVar4 = 0;
      if (local_818 != (undefined8 *)0x0) {
        uVar4 = *local_818;
      }
                    /* try { // try from 001683e8 to 001683eb has its CatchHandler @ 00168424 */
      rcutils_log(handleAutoRechargeTask(std::shared_ptr<rmw_request_id_t>,std::shared_ptr<std_srvs::srv::Trigger_Request_<std::allocator<void>>>,std::shared_ptr<std_srvs::srv::Trigger_Response_<std::allocator<void>>>)
                  ::__rcutils_logging_location,0x1e,uVar4,
                  "Cannot recharge when recharge task is executing!!!");
      if (local_810 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_810);
      }
    }
    *(undefined1 *)*param_4 = 0;
  }
  if (local_8 - *(long *)PTR___stack_chk_guard_00448f98 == 0) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail(local_8 - *(long *)PTR___stack_chk_guard_00448f98,0);
}



// ==================== RobotDecision::handleNavToRechargeTask @ 00168758 ====================

/* RobotDecision::handleNavToRechargeTask(std::shared_ptr<rmw_request_id_t>,
   std::shared_ptr<decision_msgs::srv::Charging_Request_<std::allocator<void> > >,
   std::shared_ptr<decision_msgs::srv::Charging_Response_<std::allocator<void> > >) */

void __thiscall
RobotDecision::handleNavToRechargeTask
          (RobotDecision *this,undefined8 param_2,undefined8 param_3,undefined8 *param_4)

{
  undefined *puVar1;
  char cVar2;
  int iVar3;
  undefined8 uVar4;
  size_t sVar5;
  undefined8 *local_818;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_810;
  undefined1 auStack_808 [1024];
  char acStack_408 [1024];
  long local_8;
  
  local_8 = *(long *)PTR___stack_chk_guard_00448f98;
  if ((this[0x73d] == (RobotDecision)0x0) && (this[0x736] == (RobotDecision)0x0)) {
    if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
       (iVar3 = rcutils_logging_initialize(0,0), puVar1 = PTR_stderr_00448e50, iVar3 != 0)) {
      fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:955] error initializing logging: "
             ,1,0x6e,*(FILE **)PTR_stderr_00448e50);
      rcutils_get_error_string(auStack_808);
      rcutils_get_error_string(acStack_408);
      sVar5 = strlen(acStack_408);
      fwrite(auStack_808,1,sVar5,*(FILE **)puVar1);
      fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar1);
      rcutils_reset_error();
    }
    rclcpp::Node::get_logger();
    uVar4 = 0;
    if (local_818 != (undefined8 *)0x0) {
      uVar4 = *local_818;
    }
                    /* try { // try from 00168860 to 00168863 has its CatchHandler @ 00168a20 */
    cVar2 = rcutils_logging_logger_is_enabled_for(uVar4,0x14);
    if (local_810 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_810);
    }
    if (cVar2 != '\0') {
      rclcpp::Node::get_logger();
      uVar4 = 0;
      if (local_818 != (undefined8 *)0x0) {
        uVar4 = *local_818;
      }
                    /* try { // try from 001688ac to 001688af has its CatchHandler @ 00168a40 */
      rcutils_log(handleNavToRechargeTask(std::shared_ptr<rmw_request_id_t>,std::shared_ptr<decision_msgs::srv::Charging_Request_<std::allocator<void>>>,std::shared_ptr<decision_msgs::srv::Charging_Response_<std::allocator<void>>>)
                  ::__rcutils_logging_location,0x14,uVar4,
                  "Receiving recharge task command for coverage task!!!");
      if (local_810 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_810);
      }
    }
    *(undefined4 *)(this + 0x73d) = 1;
    *(undefined1 *)*param_4 = 1;
  }
  else {
    if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
       (iVar3 = rcutils_logging_initialize(0,0), puVar1 = PTR_stderr_00448e50, iVar3 != 0)) {
      fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:952] error initializing logging: "
             ,1,0x6e,*(FILE **)PTR_stderr_00448e50);
      rcutils_get_error_string(auStack_808);
      rcutils_get_error_string(acStack_408);
      sVar5 = strlen(acStack_408);
      fwrite(auStack_808,1,sVar5,*(FILE **)puVar1);
      fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar1);
      rcutils_reset_error();
    }
    rclcpp::Node::get_logger();
    uVar4 = 0;
    if (local_818 != (undefined8 *)0x0) {
      uVar4 = *local_818;
    }
                    /* try { // try from 001687cc to 001687cf has its CatchHandler @ 00168a48 */
    cVar2 = rcutils_logging_logger_is_enabled_for(uVar4,0x1e);
    if (local_810 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_810);
    }
    if (cVar2 != '\0') {
      rclcpp::Node::get_logger();
      uVar4 = 0;
      if (local_818 != (undefined8 *)0x0) {
        uVar4 = *local_818;
      }
                    /* try { // try from 00168904 to 00168907 has its CatchHandler @ 00168a44 */
      rcutils_log(handleNavToRechargeTask(std::shared_ptr<rmw_request_id_t>,std::shared_ptr<decision_msgs::srv::Charging_Request_<std::allocator<void>>>,std::shared_ptr<decision_msgs::srv::Charging_Response_<std::allocator<void>>>)
                  ::__rcutils_logging_location,0x1e,uVar4,
                  "Cannot recharge when recharge task is executing!!!");
      if (local_810 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_810);
      }
    }
    *(undefined1 *)*param_4 = 0;
  }
  if (local_8 - *(long *)PTR___stack_chk_guard_00448f98 == 0) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail(local_8 - *(long *)PTR___stack_chk_guard_00448f98,0);
}



// ==================== RobotDecision::updateErrorStatus @ 0016be98 ====================

/* RobotDecision::updateErrorStatus(robot_status::ErrorStatus, bool) */

void __thiscall RobotDecision::updateErrorStatus(RobotDecision *this,int param_2,char param_3)

{
  bool bVar1;
  undefined4 uVar2;
  undefined *puVar3;
  char cVar4;
  int iVar5;
  undefined8 uVar6;
  size_t sVar7;
  undefined8 uVar8;
  undefined8 *local_818;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_810;
  undefined1 auStack_808 [1024];
  undefined1 *local_408 [2];
  undefined1 auStack_3f8 [1008];
  long local_8;
  
  local_8 = *(long *)PTR___stack_chk_guard_00448f98;
  iVar5 = *(int *)(this + 0x388);
  if ((iVar5 != param_2) && (iVar5 < 0x97 || param_3 != '\0')) {
    if (param_2 != 0) {
      bVar1 = iVar5 < 100;
      if (param_2 < 100 && !bVar1) goto LAB_0016bfe0;
      if ((!bVar1 && iVar5 != param_2) && (bVar1 || param_2 <= iVar5)) {
        if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
           (iVar5 = rcutils_logging_initialize(), puVar3 = PTR_stderr_00448e50, iVar5 != 0)) {
          fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:654] error initializing logging: "
                 ,1,0x6e,*(FILE **)PTR_stderr_00448e50);
          rcutils_get_error_string(auStack_808);
          rcutils_get_error_string(local_408);
          sVar7 = strlen((char *)local_408);
          fwrite(auStack_808,1,sVar7,*(FILE **)puVar3);
          fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar3);
          rcutils_reset_error();
        }
        rclcpp::Node::get_logger();
        uVar6 = 0;
        if (local_818 != (undefined8 *)0x0) {
          uVar6 = *local_818;
        }
                    /* try { // try from 0016c104 to 0016c107 has its CatchHandler @ 0016c334 */
        cVar4 = rcutils_logging_logger_is_enabled_for(uVar6,0x1e);
        if (local_810 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
          std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_810);
        }
        if (cVar4 != '\0') {
          rclcpp::Node::get_logger();
          uVar6 = 0;
          if (local_818 != (undefined8 *)0x0) {
            uVar6 = *local_818;
          }
          uVar8 = *(undefined8 *)(this + 0x398);
                    /* try { // try from 0016c148 to 0016c14b has its CatchHandler @ 0016c364 */
          robot_status::ErrorInfoMessage_abi_cxx11_((robot_status *)local_408,param_2);
                    /* try { // try from 0016c170 to 0016c173 has its CatchHandler @ 0016c33c */
          rcutils_log(updateErrorStatus(robot_status::ErrorStatus,bool)::__rcutils_logging_location,
                      0x1e,uVar6,"Error cannot overwrite: old: %s  new:%s",uVar8,local_408[0]);
          if (local_408[0] != auStack_3f8) {
            operator_delete(local_408[0]);
          }
          if (local_810 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
            std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_810);
          }
        }
      }
    }
    if ((PTR___pthread_key_create_00448e10 != (undefined *)0x0) &&
       (iVar5 = pthread_mutex_lock((pthread_mutex_t *)(this + 0x570)), iVar5 != 0)) {
                    /* WARNING: Subroutine does not return */
      std::__throw_system_error(iVar5);
    }
    uVar2 = *(undefined4 *)(this + 0x388);
    *(int *)(this + 0x388) = param_2;
    *(undefined4 *)(this + 0x394) = uVar2;
    *(int *)(this + 0x3dd) = param_2;
    if (PTR___pthread_key_create_00448e10 != (undefined *)0x0) {
      pthread_mutex_unlock((pthread_mutex_t *)(this + 0x570));
    }
    robot_status::ErrorInfoMessage_abi_cxx11_((robot_status *)local_408,param_2);
    std::__cxx11::string::operator=((string *)(this + 0x398),(string *)local_408);
    if (local_408[0] != auStack_3f8) {
      operator_delete(local_408[0]);
    }
    syncDataToPublish(this);
    if (*(int *)(this + 0x388) < 0x65) {
      if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
         (iVar5 = rcutils_logging_initialize(), puVar3 = PTR_stderr_00448e50, iVar5 != 0)) {
        fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:671] error initializing logging: "
               ,1,0x6e,*(FILE **)PTR_stderr_00448e50);
        rcutils_get_error_string(auStack_808);
        rcutils_get_error_string((string *)local_408);
        sVar7 = strlen((char *)local_408);
        fwrite(auStack_808,1,sVar7,*(FILE **)puVar3);
        fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar3);
        rcutils_reset_error();
      }
      rclcpp::Node::get_logger();
      uVar6 = 0;
      if (local_818 != (undefined8 *)0x0) {
        uVar6 = *local_818;
      }
                    /* try { // try from 0016c030 to 0016c033 has its CatchHandler @ 0016c310 */
      cVar4 = rcutils_logging_logger_is_enabled_for(uVar6,0x14);
      if (local_810 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_810);
      }
      if (cVar4 == '\0') goto LAB_0016bfe0;
      rclcpp::Node::get_logger();
      uVar6 = 0;
      if (local_818 != (undefined8 *)0x0) {
        uVar6 = *local_818;
      }
                    /* try { // try from 0016c080 to 0016c083 has its CatchHandler @ 0016c338 */
      rcutils_log(updateErrorStatus(robot_status::ErrorStatus,bool)::__rcutils_logging_location,0x14
                  ,uVar6,"%s",*(undefined8 *)(this + 0x398));
    }
    else {
      if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
         (iVar5 = rcutils_logging_initialize(), puVar3 = PTR_stderr_00448e50, iVar5 != 0)) {
        fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:669] error initializing logging: "
               ,1,0x6e,*(FILE **)PTR_stderr_00448e50);
        rcutils_get_error_string(auStack_808);
        rcutils_get_error_string((string *)local_408);
        sVar7 = strlen((char *)local_408);
        fwrite(auStack_808,1,sVar7,*(FILE **)puVar3);
        fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar3);
        rcutils_reset_error();
      }
      rclcpp::Node::get_logger();
      uVar6 = 0;
      if (local_818 != (undefined8 *)0x0) {
        uVar6 = *local_818;
      }
                    /* try { // try from 0016bfc0 to 0016bfc3 has its CatchHandler @ 0016c330 */
      cVar4 = rcutils_logging_logger_is_enabled_for(uVar6,0x1e);
      if (local_810 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_810);
      }
      if (cVar4 == '\0') goto LAB_0016bfe0;
      rclcpp::Node::get_logger();
      uVar6 = 0;
      if (local_818 != (undefined8 *)0x0) {
        uVar6 = *local_818;
      }
                    /* try { // try from 0016c0bc to 0016c0bf has its CatchHandler @ 0016c32c */
      rcutils_log(updateErrorStatus(robot_status::ErrorStatus,bool)::__rcutils_logging_location,0x1e
                  ,uVar6,"%s",*(undefined8 *)(this + 0x398));
    }
    if (local_810 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_810);
    }
  }
LAB_0016bfe0:
  if (local_8 - *(long *)PTR___stack_chk_guard_00448f98 == 0) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail(PTR___stack_chk_guard_00448f98,local_8 - *(long *)PTR___stack_chk_guard_00448f98,
                   0);
}



// ==================== RobotDecision::rechargeFinishedDeal @ 0018cfa0 ====================

/* RobotDecision::rechargeFinishedDeal() */

void RobotDecision::rechargeFinishedDeal(void)

{
  RobotDecision RVar1;
  undefined *puVar2;
  char cVar3;
  int iVar4;
  RobotDecision *in_x0;
  undefined8 uVar5;
  size_t __n;
  allocator *in_x2;
  undefined8 *local_818;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_810;
  undefined1 auStack_808 [1024];
  undefined1 *local_408 [2];
  undefined1 auStack_3f8 [1008];
  long local_8;
  
  local_8 = *(long *)PTR___stack_chk_guard_00448f98;
  if (in_x0[0x1530] == (RobotDecision)0x4) {
    if (in_x0[0x748] == (RobotDecision)0x0) {
      local_818 = (undefined8 *)&DAT_000000c8;
      std::this_thread::sleep_for<long,std::ratio<1l,1000l>>((duration *)&local_818);
      if (in_x0[0x748] == (RobotDecision)0x0) {
        if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
           (iVar4 = rcutils_logging_initialize(), puVar2 = PTR_stderr_00448e50, iVar4 != 0)) {
          fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:2698] error initializing logging: "
                 ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
          rcutils_get_error_string(auStack_808);
          rcutils_get_error_string(local_408);
          __n = strlen((char *)local_408);
          fwrite(auStack_808,1,__n,*(FILE **)puVar2);
          fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar2);
          rcutils_reset_error();
        }
        rclcpp::Node::get_logger();
        uVar5 = 0;
        if (local_818 != (undefined8 *)0x0) {
          uVar5 = *local_818;
        }
                    /* try { // try from 0018d0ec to 0018d0ef has its CatchHandler @ 0018d2c0 */
        cVar3 = rcutils_logging_logger_is_enabled_for(uVar5,0x1e);
        if (local_810 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
          std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_810);
        }
        if (cVar3 != '\0') {
          rclcpp::Node::get_logger();
          uVar5 = 0;
          if (local_818 != (undefined8 *)0x0) {
            uVar5 = *local_818;
          }
                    /* try { // try from 0018d138 to 0018d13b has its CatchHandler @ 0018d2a4 */
          rcutils_log(rechargeFinishedDeal()::__rcutils_logging_location,0x1e,uVar5,
                      "Robot is not in charging, but recharging action finished, Todo to try again!!"
                     );
          if (local_810 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
            std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_810);
          }
        }
        updateWorkStatus(in_x0,(uchar)in_x0[900],'\x01');
        updateErrorStatus();
        goto LAB_0018d064;
      }
    }
    updateWorkStatus(in_x0,(uchar)in_x0[900],'\t');
  }
  else {
    std::__cxx11::string::string<std::allocator<char>>((string *)local_408,"recharge_fail",in_x2);
                    /* try { // try from 0018cff8 to 0018cffb has its CatchHandler @ 0018d27c */
    saveImage();
    if (local_408[0] != auStack_3f8) {
      operator_delete(local_408[0]);
    }
    RVar1 = in_x0[900];
    if (in_x0[0x1530] == (RobotDecision)0x5) {
      updateTaskType();
      updateWorkStatus(in_x0,(uchar)RVar1,'\x02');
    }
    else if (*(char *)(*(long *)(in_x0 + 0x1538) + 0x21) == '\x03') {
      updateTaskType();
      updateWorkStatus(in_x0,(uchar)RVar1,'\x01');
      updateErrorStatus();
    }
    else if (*(char *)(*(long *)(in_x0 + 0x1538) + 0x21) == '\v') {
      updateTaskType();
      updateWorkStatus(in_x0,(uchar)RVar1,'\x01');
      updateErrorStatus();
    }
    else {
      updateTaskType();
      updateWorkStatus(in_x0,(uchar)RVar1,'\x01');
      updateErrorStatus();
    }
  }
LAB_0018d064:
  if (local_8 - *(long *)PTR___stack_chk_guard_00448f98 == 0) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail(PTR___stack_chk_guard_00448f98,local_8 - *(long *)PTR___stack_chk_guard_00448f98,
                   0);
}



// ==================== RobotDecision::rechargeDeal @ 0018dfb8 ====================

/* RobotDecision::rechargeDeal(bool) */

void __thiscall RobotDecision::rechargeDeal(RobotDecision *this,bool param_1)

{
  __shared_count *p_Var1;
  undefined *puVar2;
  char cVar3;
  int iVar4;
  undefined8 uVar5;
  long lVar6;
  size_t sVar7;
  RobotDecision RVar8;
  undefined1 local_830 [8];
  undefined8 local_828;
  _State_baseV2 *local_820;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_818;
  undefined8 local_810;
  undefined1 *local_808;
  undefined1 auStack_7f8 [16];
  undefined8 local_7e8;
  undefined8 uStack_7e0;
  undefined8 local_7d8;
  undefined8 uStack_7d0;
  undefined8 local_7c8;
  undefined8 uStack_7c0;
  undefined8 local_7b8;
  undefined8 local_410;
  undefined1 *local_408;
  code *local_400;
  undefined1 auStack_3f8 [16];
  undefined8 local_3e8;
  undefined8 uStack_3e0;
  undefined8 local_3d8;
  undefined8 uStack_3d0;
  undefined8 local_3c8;
  undefined8 uStack_3c0;
  undefined8 local_3b8;
  undefined4 local_3b0;
  undefined2 local_3ac;
  undefined1 local_3aa;
  long local_8;
  
  local_8 = *(long *)PTR___stack_chk_guard_00448f98;
  cVar3 = rclcpp_action::ClientBase::wait_for_action_server_nanoseconds
                    (*(undefined8 *)(this + 0x12d0),1000000000,PTR___stack_chk_guard_00448f98,0);
  if (cVar3 == '\0') {
    updateWorkStatus(this,(uchar)this[900],'\x01');
    updateErrorStatus(this,0x6f,0);
    goto LAB_0018e0a8;
  }
  if (0x59 < (byte)this[0x385]) {
    if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
       (iVar4 = rcutils_logging_initialize(), puVar2 = PTR_stderr_00448e50, iVar4 != 0)) {
      fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:3032] error initializing logging: "
             ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
      rcutils_get_error_string(&local_810);
      rcutils_get_error_string(&local_410);
      sVar7 = strlen((char *)&local_410);
      fwrite(&local_810,1,sVar7,*(FILE **)puVar2);
      fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar2);
      rcutils_reset_error();
    }
    rclcpp::Node::get_logger();
    uVar5 = 0;
    if (local_820 != (_State_baseV2 *)0x0) {
      uVar5 = *(undefined8 *)local_820;
    }
                    /* try { // try from 0018e03c to 0018e03f has its CatchHandler @ 0018ec04 */
    cVar3 = rcutils_logging_logger_is_enabled_for(uVar5,0x1e);
    if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
    }
    if (cVar3 != '\0') {
      rclcpp::Node::get_logger();
      uVar5 = 0;
      if (local_820 != (_State_baseV2 *)0x0) {
        uVar5 = *(undefined8 *)local_820;
      }
                    /* try { // try from 0018e088 to 0018e08b has its CatchHandler @ 0018eb74 */
      rcutils_log(rechargeDeal(bool)::__rcutils_logging_location,0x1e,uVar5,
                  "Already in recharging status, No need to recharge!!!");
      if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
      }
    }
    updateErrorStatus(this,2,0);
    goto LAB_0018e0a8;
  }
  if (0x95 < *(int *)(this + 0x388)) {
    if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
       (iVar4 = rcutils_logging_initialize(), puVar2 = PTR_stderr_00448e50, iVar4 != 0)) {
      fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:3037] error initializing logging: "
             ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
      rcutils_get_error_string(&local_810);
      rcutils_get_error_string(&local_410);
      sVar7 = strlen((char *)&local_410);
      fwrite(&local_810,1,sVar7,*(FILE **)puVar2);
      fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar2);
      rcutils_reset_error();
    }
    rclcpp::Node::get_logger();
    uVar5 = 0;
    if (local_820 != (_State_baseV2 *)0x0) {
      uVar5 = *(undefined8 *)local_820;
    }
                    /* try { // try from 0018e110 to 0018e113 has its CatchHandler @ 0018eb98 */
    cVar3 = rcutils_logging_logger_is_enabled_for(uVar5,0x1e);
    if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
    }
    if (cVar3 != '\0') {
      rclcpp::Node::get_logger();
      uVar5 = 0;
      if (local_820 != (_State_baseV2 *)0x0) {
        uVar5 = *(undefined8 *)local_820;
      }
                    /* try { // try from 0018e15c to 0018e15f has its CatchHandler @ 0018eb9c */
      rcutils_log(rechargeDeal(bool)::__rcutils_logging_location,0x1e,uVar5,
                  "Robot seems to have some error, please unlock to retry again");
      if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
      }
    }
LAB_0018e16c:
    RVar8 = this[900];
LAB_0018e170:
    updateWorkStatus(this,(uchar)RVar8,'\x01');
    goto LAB_0018e0a8;
  }
  if (this[0x748] != (RobotDecision)0x0) {
    updateWorkStatus(this,(uchar)this[900],'\t');
    goto LAB_0018e0a8;
  }
  if (this[0x759] != (RobotDecision)0x0) {
    updateErrorStatus(this,0x85,0);
    goto LAB_0018e16c;
  }
  if (((*(uint *)(this + 0x380) & 0xfffffffd) == 1) && (0x59 < (byte)this[900])) {
    coverStopDeal(this,'\v');
  }
  setSemanticMode(this);
  updateErrorStatus(this,0,0);
  updateWorkStatus(this,(uchar)this[900],'2');
  local_820 = (_State_baseV2 *)&DAT_00000190;
  std::this_thread::sleep_for<long,std::ratio<1l,1000l>>((duration *)&local_820);
  checkChassisStatus(this);
  checkLocalizationStatus(this);
  RVar8 = this[900];
  if (99 < *(int *)(this + 0x388)) goto LAB_0018e170;
  updateWorkStatus(this,(uchar)RVar8,'9');
  if (this[0x810] != (RobotDecision)0x0) {
    updateWorkStatus(this,(uchar)this[900],'\x01');
    updateErrorStatus(this,0x88,0);
    goto LAB_0018e0a8;
  }
  cVar3 = rclcpp_action::ClientBase::wait_for_action_server_nanoseconds
                    (*(undefined8 *)(this + 0x14a8),1000000000);
  if (cVar3 == '\0') {
    updateWorkStatus(this,(uchar)this[900],'\x01');
    updateErrorStatus(this,0x7a,0);
    goto LAB_0018e0a8;
  }
  updateWorkStatus(this,(uchar)this[900],'7');
  if (((this[0x602] == (RobotDecision)0x0) &&
      ((this[0x6b2] == (RobotDecision)0x0 || (cVar3 = locInit(this), cVar3 == '\0')))) ||
     ((updateWorkStatus(this,(uchar)this[900],'3'), this[0x602] == (RobotDecision)0x0 &&
      (cVar3 = sensorInit(this), cVar3 == '\0')))) goto LAB_0018e16c;
  if (param_1) {
    updateTaskType(this,2);
    updateWorkStatus(this,'\0','2');
  }
  if (this[0x6e7] != (RobotDecision)0x0) {
    if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
       (iVar4 = rcutils_logging_initialize(), puVar2 = PTR_stderr_00448e50, iVar4 != 0)) {
      fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:3091] error initializing logging: "
             ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
      rcutils_get_error_string(&local_810);
      rcutils_get_error_string(&local_410);
      sVar7 = strlen((char *)&local_410);
      fwrite(&local_810,1,sVar7,*(FILE **)puVar2);
      fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar2);
      rcutils_reset_error();
    }
    rclcpp::Node::get_logger();
    uVar5 = 0;
    if (local_820 != (_State_baseV2 *)0x0) {
      uVar5 = *(undefined8 *)local_820;
    }
                    /* try { // try from 0018e320 to 0018e323 has its CatchHandler @ 0018ec0c */
    cVar3 = rcutils_logging_logger_is_enabled_for(uVar5,0x1e);
    if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
    }
    if (cVar3 != '\0') {
      rclcpp::Node::get_logger();
      uVar5 = 0;
      if (local_820 != (_State_baseV2 *)0x0) {
        uVar5 = *(undefined8 *)local_820;
      }
                    /* try { // try from 0018e36c to 0018e36f has its CatchHandler @ 0018ec08 */
      rcutils_log(rechargeDeal(bool)::__rcutils_logging_location,0x1e,uVar5,
                  "Already in recharging status, just cancel current task!!!");
      if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
      }
    }
    local_400 = (code *)0x0;
                    /* try { // try from 0018e390 to 0018e393 has its CatchHandler @ 0018ebd8 */
    rclcpp_action::Client<automatic_recharge_msgs::action::AutoCharging>::async_cancel_all_goals
              ((duration *)&local_820,*(undefined8 *)(this + 0x12d0),&local_410);
    if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
    }
    if (local_400 != (code *)0x0) {
      (*local_400)(&local_410,&local_410,3);
    }
    local_820 = (_State_baseV2 *)&DAT_000001f4;
    std::this_thread::sleep_for<long,std::ratio<1l,1000l>>((duration *)&local_820);
  }
  if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
     (iVar4 = rcutils_logging_initialize(), puVar2 = PTR_stderr_00448e50, iVar4 != 0)) {
    fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:3095] error initializing logging: "
           ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
    rcutils_get_error_string(&local_810);
    rcutils_get_error_string(&local_410);
    sVar7 = strlen((char *)&local_410);
    fwrite(&local_810,1,sVar7,*(FILE **)puVar2);
    fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar2);
    rcutils_reset_error();
  }
  rclcpp::Node::get_logger();
  uVar5 = 0;
  if (local_820 != (_State_baseV2 *)0x0) {
    uVar5 = *(undefined8 *)local_820;
  }
                    /* try { // try from 0018e3f0 to 0018e3f3 has its CatchHandler @ 0018ebfc */
  cVar3 = rcutils_logging_logger_is_enabled_for(uVar5,0x14);
  if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
    std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
  }
  if (cVar3 != '\0') {
    rclcpp::Node::get_logger();
    uVar5 = 0;
    if (local_820 != (_State_baseV2 *)0x0) {
      uVar5 = *(undefined8 *)local_820;
    }
                    /* try { // try from 0018e5cc to 0018e5cf has its CatchHandler @ 0018eba0 */
    rcutils_log(rechargeDeal(bool)::__rcutils_logging_location,0x14,uVar5,
                "Start to recharge with guide pose mode:%d power: %d%%",!param_1,this[0x75a]);
    if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
    }
  }
  if (!param_1) {
    if (*(int *)(this + 0x380) == 2) {
      if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
         (iVar4 = rcutils_logging_initialize(), puVar2 = PTR_stderr_00448e50, iVar4 != 0)) {
        fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:3100] error initializing logging: "
               ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
        rcutils_get_error_string(&local_810);
        rcutils_get_error_string(&local_410);
        sVar7 = strlen((char *)&local_410);
        fwrite(&local_810,1,sVar7,*(FILE **)puVar2);
        fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar2);
        rcutils_reset_error();
      }
      rclcpp::Node::get_logger();
      uVar5 = 0;
      if (local_820 != (_State_baseV2 *)0x0) {
        uVar5 = *(undefined8 *)local_820;
      }
                    /* try { // try from 0018e814 to 0018e817 has its CatchHandler @ 0018ebd4 */
      cVar3 = rcutils_logging_logger_is_enabled_for(uVar5,0x14);
      if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
      }
      if (cVar3 != '\0') {
        rclcpp::Node::get_logger();
        uVar5 = 0;
        if (local_820 != (_State_baseV2 *)0x0) {
          uVar5 = *(undefined8 *)local_820;
        }
                    /* try { // try from 0018e860 to 0018e863 has its CatchHandler @ 0018ec00 */
        rcutils_log(rechargeDeal(bool)::__rcutils_logging_location,0x14,uVar5,
                    "Recharge with guide pose mode only support no mapping mode ");
        if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
          std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
        }
      }
      RVar8 = this[0x385];
      updateTaskType(this,1);
      updateWorkStatus(this,'\0',(uchar)RVar8);
    }
    updateWorkStatus(this,(uchar)this[900],'5');
    cVar3 = loadMap(this);
    if (cVar3 != '\0') {
      if (this[0x7ec] == (RobotDecision)0x0) {
        setPerceptionLevel(this);
        goto LAB_0018e454;
      }
      updateErrorStatus(this,0x7c,0);
    }
    updateWorkStatus(this,(uchar)this[900],'\x01');
    goto LAB_0018e0a8;
  }
  updateWorkStatus(this,(uchar)this[900],'5');
LAB_0018e454:
  std_msgs::msg::Header_<std::allocator<void>>::Header_((SendGoalOptions *)&local_410);
  local_3b0 = 0;
  local_3ac = 0;
  local_3aa = 0;
  local_3b8 = 0x3ff0000000000000;
  uStack_3e0 = 0;
  local_3e8 = 0;
  uStack_3d0 = 0;
  local_3d8 = 0;
  uStack_3c0 = 0;
  local_3c8 = 0;
  if (param_1) {
    local_3b0 = 0x5000100;
LAB_0018e49c:
    local_830[0] = 0x5a;
                    /* try { // try from 0018e4b4 to 0018e4cf has its CatchHandler @ 0018eb34 */
    (**(code **)(**(long **)(this + 0x1290) + 0x20))(*(long **)(this + 0x1290),local_830);
    rclcpp_action::Client<automatic_recharge_msgs::action::AutoCharging>::async_send_goal
              (*(AutoCharging_Goal_ **)(this + 0x12d0),(SendGoalOptions *)&local_410);
    if (local_820 == (_State_baseV2 *)0x0) {
LAB_0018eb24:
                    /* try { // try from 0018eb28 to 0018eb2b has its CatchHandler @ 0018eb2c */
      uVar5 = std::__throw_future_error(3);
                    /* catch() { ... } // from try @ 0018e4d8 with catch @ 0018eb2c
                       catch() { ... } // from try @ 0018eb28 with catch @ 0018eb2c */
      if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
      }
      if (local_408 != auStack_3f8) {
        operator_delete(local_408);
      }
                    /* WARNING: Subroutine does not return */
      _Unwind_Resume(uVar5);
    }
                    /* try { // try from 0018e4d8 to 0018e4db has its CatchHandler @ 0018eb2c */
    lVar6 = std::__future_base::_State_baseV2::wait(local_820);
    local_828 = 0;
    cVar3 = std::__exception_ptr::operator==
                      ((exception_ptr *)(lVar6 + 8),(exception_ptr *)&local_828);
    std::__exception_ptr::exception_ptr::~exception_ptr(&local_828);
    if (cVar3 == '\0') {
      std::__exception_ptr::exception_ptr::exception_ptr(&local_828,(exception_ptr *)(lVar6 + 8));
                    /* try { // try from 0018eb20 to 0018eb23 has its CatchHandler @ 0018eb58 */
      std::rethrow_exception(&local_828);
      goto LAB_0018eb24;
    }
    p_Var1 = *(__shared_count **)(lVar6 + 0x18);
    *(undefined8 *)(this + 0x1470) = *(undefined8 *)(lVar6 + 0x10);
    std::__shared_count<(__gnu_cxx::_Lock_policy)2>::operator=
              ((__shared_count<(__gnu_cxx::_Lock_policy)2> *)(this + 0x1478),p_Var1);
    if (local_818 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_818);
    }
    *(undefined4 *)(this + 0x155b) = 0;
    *(undefined4 *)(this + 0x155a) = 0;
    if (*(long *)(this + 0x1470) == 0) {
                    /* try { // try from 0018e8a0 to 0018e8b3 has its CatchHandler @ 0018eb34 */
      updateWorkStatus(this,(uchar)this[900],'\x01');
      updateErrorStatus(this,0x6f,0);
    }
    else {
      *(undefined4 *)(this + 0x6e7) = 1;
      RVar8 = this[900];
                    /* try { // try from 0018e558 to 0018e56b has its CatchHandler @ 0018eb34 */
      updateTaskType(this,*(undefined4 *)(this + 0x380));
      updateWorkStatus(this,(uchar)RVar8,0xbf);
    }
  }
  else {
                    /* try { // try from 0018e7a4 to 0018e7a7 has its CatchHandler @ 0018eb34 */
    std_msgs::msg::Header_<std::allocator<void>>::Header_((PoseStamped_ *)&local_810);
    local_7b8 = 0x3ff0000000000000;
    uStack_7e0 = 0;
    local_7e8 = 0;
    uStack_7d0 = 0;
    local_7d8 = 0;
    uStack_7c0 = 0;
    local_7c8 = 0;
    if (this[0x602] != (RobotDecision)0x0) {
      local_3b0._0_3_ = CONCAT12(1,(undefined2)local_3b0);
      local_3ac = CONCAT11(local_3ac._1_1_,1);
LAB_0018e7d4:
      if (local_808 != auStack_7f8) {
        operator_delete(local_808);
      }
      goto LAB_0018e49c;
    }
                    /* try { // try from 0018e970 to 0018e98f has its CatchHandler @ 0018ebb4 */
    cVar3 = getChargingPose(this,(PoseStamped_ *)&local_810);
    if (cVar3 != '\0') {
      local_410 = local_810;
      std::__cxx11::string::_M_assign((string *)&local_408);
      local_3e8 = local_7e8;
      uStack_3e0 = uStack_7e0;
      local_3d8 = local_7d8;
      uStack_3d0 = uStack_7d0;
      local_3c8 = local_7c8;
      uStack_3c0 = uStack_7c0;
      local_3b8 = local_7b8;
      local_3b0 = CONCAT31(local_3b0._1_3_,1);
      local_3b0 = CONCAT13(10,(undefined3)local_3b0);
      goto LAB_0018e7d4;
    }
                    /* try { // try from 0018ea68 to 0018ea7b has its CatchHandler @ 0018ebb4 */
    updateErrorStatus(this,0x70,0);
    updateWorkStatus(this,(uchar)this[900],'\x01');
    if (local_808 != auStack_7f8) {
      operator_delete(local_808);
    }
  }
  if (local_408 != auStack_3f8) {
    operator_delete(local_408);
  }
LAB_0018e0a8:
  if (local_8 - *(long *)PTR___stack_chk_guard_00448f98 != 0) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail(PTR___stack_chk_guard_00448f98,
                     local_8 - *(long *)PTR___stack_chk_guard_00448f98,0);
  }
  return;
}



// ==================== RobotDecision::coverStartDeal @ 00191a28 ====================

/* RobotDecision::coverStartDeal() */

void __thiscall RobotDecision::coverStartDeal(RobotDecision *this)

{
  __shared_count *p_Var1;
  undefined *puVar2;
  char cVar3;
  int iVar4;
  uint uVar5;
  undefined8 uVar6;
  size_t sVar7;
  long lVar8;
  char *pcVar9;
  ulong uVar10;
  char *pcVar11;
  char *this_00;
  exception_ptr *unaff_x27;
  RobotDecision local_868 [8];
  char local_860 [8];
  _State_baseV2 *local_858;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_850;
  undefined1 *local_848 [2];
  undefined1 auStack_838 [16];
  undefined1 local_828;
  undefined2 local_827;
  undefined1 local_825;
  undefined1 local_822;
  undefined1 local_820;
  char local_81f;
  undefined4 local_81c;
  undefined1 local_817;
  char local_816;
  char local_815;
  undefined1 auStack_808 [1024];
  char acStack_408 [1024];
  long local_8;
  
  iVar4 = *(int *)(this + 0x380);
  local_8 = *(long *)PTR___stack_chk_guard_00448f98;
  if (iVar4 == 2) {
    if (0x59 < (byte)this[900]) {
      if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
         (iVar4 = rcutils_logging_initialize(0,PTR___stack_chk_guard_00448f98,0),
         puVar2 = PTR_stderr_00448e50, iVar4 != 0)) {
        fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:1837] error initializing logging: "
               ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
        rcutils_get_error_string(auStack_808);
        rcutils_get_error_string(acStack_408);
        sVar7 = strlen(acStack_408);
        fwrite(auStack_808,1,sVar7,*(FILE **)puVar2);
        fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar2);
        rcutils_reset_error();
      }
      rclcpp::Node::get_logger();
      uVar6 = 0;
      if (local_858 != (_State_baseV2 *)0x0) {
        uVar6 = *(undefined8 *)local_858;
      }
                    /* try { // try from 00191bb4 to 00191bb7 has its CatchHandler @ 00192854 */
      cVar3 = rcutils_logging_logger_is_enabled_for(uVar6,0x28);
      if (local_850 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_850);
      }
      if (cVar3 != '\0') {
        rclcpp::Node::get_logger();
        uVar6 = 0;
        if (local_858 != (_State_baseV2 *)0x0) {
          uVar6 = *(undefined8 *)local_858;
        }
                    /* try { // try from 00191e10 to 00191e13 has its CatchHandler @ 00192868 */
        rcutils_log(coverStartDeal()::__rcutils_logging_location,0x28,uVar6,
                    "In mapping status, please check your operation is right!!!");
        if (local_850 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
          std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_850);
          iVar4 = *(int *)(this + 0x380);
          goto LAB_00191a68;
        }
      }
      iVar4 = *(int *)(this + 0x380);
      goto LAB_00191a68;
    }
  }
  else {
LAB_00191a68:
    if ((iVar4 == 1) && (0x59 < (byte)this[900])) {
      updateErrorStatus(this,2,0);
      goto LAB_00191b24;
    }
  }
  if (0x95 < *(int *)(this + 0x388)) {
    if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
       (iVar4 = rcutils_logging_initialize(), puVar2 = PTR_stderr_00448e50, iVar4 != 0)) {
      fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:1844] error initializing logging: "
             ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
      rcutils_get_error_string(auStack_808);
      rcutils_get_error_string(acStack_408);
      sVar7 = strlen(acStack_408);
      fwrite(auStack_808,1,sVar7,*(FILE **)puVar2);
      fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar2);
      rcutils_reset_error();
    }
    rclcpp::Node::get_logger();
    uVar6 = 0;
    if (local_858 != (_State_baseV2 *)0x0) {
      uVar6 = *(undefined8 *)local_858;
    }
                    /* try { // try from 00191aac to 00191aaf has its CatchHandler @ 001927f4 */
    cVar3 = rcutils_logging_logger_is_enabled_for(uVar6,0x1e);
    if (local_850 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
      std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_850);
    }
    if (cVar3 != '\0') {
      rclcpp::Node::get_logger();
      uVar6 = 0;
      if (local_858 != (_State_baseV2 *)0x0) {
        uVar6 = *(undefined8 *)local_858;
      }
                    /* try { // try from 00191af8 to 00191afb has its CatchHandler @ 001927d8 */
      rcutils_log(coverStartDeal()::__rcutils_logging_location,0x1e,uVar6,
                  "Robot seems to have some error, please unlock to retry again");
      if (local_850 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
        std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_850);
      }
    }
    updateTaskType(this,1);
    updateWorkStatus(this,'\x01','\0');
    goto LAB_00191b24;
  }
  updateErrorStatus(this,0,0);
  updateTaskType(this,1);
  updateWorkStatus(this,'2','\0');
  *(undefined4 *)(this + 0x6e6) = 0;
  local_858 = (_State_baseV2 *)&DAT_00000190;
  std::this_thread::sleep_for<long,std::ratio<1l,1000l>>((duration *)&local_858);
  checkChassisStatus(this);
  checkLocalizationStatus(this);
  if (99 < *(int *)(this + 0x388)) goto LAB_00191dcc;
  if (this[0x759] == (RobotDecision)0x0) {
    if ((uint)(byte)this[0x75a] <= (byte)this[0x600] + 1) {
      uVar6 = 0x77;
      goto LAB_00191dc4;
    }
    updateWorkStatus(this,'9','\0');
    if (this[0x810] != (RobotDecision)0x0) {
      updateWorkStatus(this,'\x01','\0');
      updateErrorStatus(this,0x88,0);
      goto LAB_00191b24;
    }
    cVar3 = rclcpp_action::ClientBase::wait_for_action_server_nanoseconds
                      (*(undefined8 *)(this + 0x14a8),1000000000);
    if (cVar3 == '\0') {
      updateWorkStatus(this,'\x01','\0');
      updateErrorStatus(this,0x7a,0);
      goto LAB_00191b24;
    }
    updateWorkStatus(this,'6','\0');
    cVar3 = loadUtmInfo(this);
    if ((cVar3 != '\0') &&
       ((updateWorkStatus(this,'3','\0'), this[0x602] != (RobotDecision)0x0 ||
        (cVar3 = sensorInit(this), cVar3 != '\0')))) {
      updateWorkStatus(this,'8','\0');
      if (this[0x748] != (RobotDecision)0x0) {
        *(undefined8 *)(this + 0x798) = 0;
        *(undefined4 *)(this + 0x7a0) = 0;
      }
      cVar3 = quitPileDeal(this);
      if (cVar3 != '\0') {
        if (((this[0x6b1] != (RobotDecision)0x0) && (this[0x6e5] != (RobotDecision)0x0)) &&
           (cVar3 = checkCameraClean(this), cVar3 == '\0')) {
          updateErrorStatus(this,0x81,0);
          updateWorkStatus(this,'\x01','\0');
          goto LAB_00191b24;
        }
        *(undefined4 *)(this + 0x6e5) = 0;
        updateWorkStatus(this,'7','\0');
        (**(code **)(**(long **)(this + 0x1270) + 0x20))(*(long **)(this + 0x1270),this + 0x500);
        if ((this[0x602] == (RobotDecision)0x0) &&
           ((this[0x6b2] == (RobotDecision)0x0 || (cVar3 = locInit(this), cVar3 == '\0')))) {
          if (this[0x732] != (RobotDecision)0x0) {
            updateWorkStatus(this,'\n','\0');
            goto LAB_00191b24;
          }
          if (this[0x734] != (RobotDecision)0x0) {
            updateWorkStatus(this,'\x02','\0');
            *(undefined4 *)(this + 0x734) = 0;
            goto LAB_00191b24;
          }
          goto LAB_00191dcc;
        }
        coverage_planner::action::NavigateThroughCoveragePaths_Goal_<std::allocator<void>>::
        NavigateThroughCoveragePaths_Goal_
                  ((NavigateThroughCoveragePaths_Goal_<std::allocator<void>> *)local_848,0);
        pcVar11 = *(char **)(this + 0x3f0);
        iVar4 = 0;
        pcVar9 = *(char **)(this + 0x3f8);
        this_00 = pcVar11;
        if (pcVar11 == pcVar9) {
          if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
             (iVar4 = rcutils_logging_initialize(), iVar4 != 0)) {
            fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:1937] error initializing logging: "
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
          if (local_858 != (_State_baseV2 *)0x0) {
            uVar6 = *(undefined8 *)local_858;
          }
                    /* try { // try from 00192448 to 0019244b has its CatchHandler @ 00192870 */
          cVar3 = rcutils_logging_logger_is_enabled_for(uVar6,0x28);
          if (local_850 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
            std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_850);
          }
          if (cVar3 != '\0') {
                    /* try { // try from 0019246c to 0019246f has its CatchHandler @ 00192844 */
            rclcpp::Node::get_logger();
            uVar6 = 0;
            if (local_858 != (_State_baseV2 *)0x0) {
              uVar6 = *(undefined8 *)local_858;
            }
                    /* try { // try from 00192494 to 00192497 has its CatchHandler @ 00192864 */
            rcutils_log(coverStartDeal()::__rcutils_logging_location,0x28,uVar6,&DAT_003742b0);
LAB_001920f4:
            if (local_850 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
              std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_850);
            }
          }
LAB_00192100:
                    /* try { // try from 0019210c to 0019210f has its CatchHandler @ 00192844 */
          updateWorkStatus(this,'\x01','\0');
        }
        else {
          do {
            if (*this_00 == '\0') {
              uVar10 = ((long)pcVar9 - (long)pcVar11 >> 3) * -0x79435e50d79435e5;
              if (uVar10 < (ulong)(long)iVar4 || uVar10 - (long)iVar4 == 0) {
                std::__throw_out_of_range_fmt
                          ("vector::_M_range_check: __n (which is %zu) >= this->size() (which is %zu)"
                           ,(long)iVar4);
                goto LAB_001927bc;
              }
                    /* try { // try from 00192180 to 00192217 has its CatchHandler @ 00192844 */
              CovTaskInfo::operator=
                        ((CovTaskInfo *)(this + 0x430),(CovTaskInfo *)(pcVar11 + (long)iVar4 * 0x98)
                        );
              local_827 = 1;
              local_822 = 1;
              local_820 = 1;
              local_81c = 1;
              local_81f = this_00[0x50] * '\n' + '\x14';
              local_817 = 1;
              std::__cxx11::string::_M_assign((string *)local_848);
              local_816 = this_00[0x90];
              local_815 = this_00[0x92];
              local_825 = 1;
              iVar4 = std::__cxx11::string::compare(this_00 + 0x30);
              if (iVar4 == 0) {
                local_828 = 6;
                if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
                   (iVar4 = rcutils_logging_initialize(), iVar4 != 0)) {
                  fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:1957] error initializing logging: "
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
                if (local_858 != (_State_baseV2 *)0x0) {
                  uVar6 = *(undefined8 *)local_858;
                }
                    /* try { // try from 00192228 to 0019222b has its CatchHandler @ 0019282c */
                cVar3 = rcutils_logging_logger_is_enabled_for(uVar6,0x1e);
                if (local_850 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
                  std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_850);
                }
                if (cVar3 != '\0') {
                    /* try { // try from 00192538 to 0019253b has its CatchHandler @ 00192844 */
                  rclcpp::Node::get_logger();
                  uVar6 = 0;
                  if (local_858 != (_State_baseV2 *)0x0) {
                    uVar6 = *(undefined8 *)local_858;
                  }
                    /* try { // try from 00192560 to 00192563 has its CatchHandler @ 00192874 */
                  rcutils_log(coverStartDeal()::__rcutils_logging_location,0x1e,uVar6,
                              "VISION_TEST!!!");
                  if (local_850 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
                    std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_850);
                  }
                }
                this[0x4c3] = (RobotDecision)0x3;
              }
              break;
            }
            this_00 = this_00 + 0x98;
            iVar4 = iVar4 + 1;
          } while (pcVar9 != this_00);
                    /* try { // try from 00192014 to 00192097 has its CatchHandler @ 00192844 */
          updateWorkStatus(this,'5','\0');
          cVar3 = loadMap(this);
          if (cVar3 == '\0') goto LAB_00192100;
          iVar4 = std::__cxx11::string::compare((char *)(this + 0x460));
          if (iVar4 == 0) {
LAB_001922a4:
                    /* try { // try from 001922b0 to 0019230b has its CatchHandler @ 00192844 */
            updateWorkStatus(this,';','\0');
            cVar3 = rclcpp_action::ClientBase::wait_for_action_server_nanoseconds
                              (*(undefined8 *)(this + 0x12c0),1000000000);
            if (cVar3 != '\0') {
              local_858 = (_State_baseV2 *)&DAT_000000c8;
              std::this_thread::sleep_for<long,std::ratio<1l,1000l>>((duration *)&local_858);
              setPerceptionLevel(this);
              pubSlippingOrCollisionPoint(this,3);
              if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
                 (iVar4 = rcutils_logging_initialize(), iVar4 != 0)) {
                fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:2002] error initializing logging: "
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
              if (local_858 != (_State_baseV2 *)0x0) {
                uVar6 = *(undefined8 *)local_858;
              }
                    /* try { // try from 0019231c to 0019231f has its CatchHandler @ 0019284c */
              uVar5 = rcutils_logging_logger_is_enabled_for(uVar6,0x14);
              this_00 = (char *)(ulong)(uVar5 & 0xff);
              if (local_850 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
                std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_850);
              }
              if ((uVar5 & 0xff) != 0) {
                rclcpp::Node::get_logger();
                uVar6 = 0;
                if (local_858 != (_State_baseV2 *)0x0) {
                  uVar6 = *(undefined8 *)local_858;
                }
                    /* try { // try from 00192514 to 00192517 has its CatchHandler @ 0019286c */
                rcutils_log(coverStartDeal()::__rcutils_logging_location,0x14,uVar6,
                            "Start task:%s  height: %d",local_848[0],local_81f);
                if (local_850 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
                  std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_850);
                }
              }
              local_868[0] = this[0x4c4];
                    /* try { // try from 0019234c to 00192367 has its CatchHandler @ 00192844 */
              (**(code **)(**(long **)(this + 0x1250) + 0x20))(*(long **)(this + 0x1250),local_868);
              rclcpp_action::Client<coverage_planner::action::NavigateThroughCoveragePaths>::
              async_send_goal(*(NavigateThroughCoveragePaths_Goal_ **)(this + 0x12c0),
                              (SendGoalOptions *)local_848);
              if (local_858 == (_State_baseV2 *)0x0) {
LAB_001927bc:
                    /* try { // try from 001927c0 to 001927c3 has its CatchHandler @ 00192858 */
                std::__throw_future_error(3);
LAB_001927c4:
                std::__exception_ptr::exception_ptr::exception_ptr(this_00,unaff_x27);
                    /* try { // try from 001927d4 to 001927d7 has its CatchHandler @ 001927f8 */
                uVar6 = std::rethrow_exception(this_00);
                    /* catch() { ... } // from try @ 00191af8 with catch @ 001927d8 */
                if (local_850 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
                  std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_850);
                }
                    /* WARNING: Subroutine does not return */
                _Unwind_Resume(uVar6);
              }
                    /* try { // try from 00192370 to 00192373 has its CatchHandler @ 00192858 */
              lVar8 = std::__future_base::_State_baseV2::wait(local_858);
              this_00 = local_860;
              unaff_x27 = (exception_ptr *)(lVar8 + 8);
              local_860[0] = '\0';
              local_860[1] = '\0';
              local_860[2] = '\0';
              local_860[3] = '\0';
              local_860[4] = '\0';
              local_860[5] = '\0';
              local_860[6] = '\0';
              local_860[7] = '\0';
              cVar3 = std::__exception_ptr::operator==(unaff_x27,this_00);
              std::__exception_ptr::exception_ptr::~exception_ptr(this_00);
              if (cVar3 == '\0') goto LAB_001927c4;
              p_Var1 = *(__shared_count **)(lVar8 + 0x18);
              *(undefined8 *)(this + 0x1460) = *(undefined8 *)(lVar8 + 0x10);
              std::__shared_count<(__gnu_cxx::_Lock_policy)2>::operator=
                        ((__shared_count<(__gnu_cxx::_Lock_policy)2> *)(this + 0x1468),p_Var1);
              if (local_850 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
                std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_850);
              }
              if (*(long *)(this + 0x1460) != 0) {
                *(undefined4 *)(this + 0x1558) = 0;
                *(undefined4 *)(this + 0x1559) = 0;
                *(undefined4 *)(this + 0x6d8) = 0;
                this[0x730] = (RobotDecision)0x0;
                *(undefined4 *)(this + 0x6e6) = 1;
                local_858 = (_State_baseV2 *)0x1;
                    /* try { // try from 001923f8 to 00192437 has its CatchHandler @ 00192844 */
                std::this_thread::sleep_for<long,std::ratio<1l,1l>>((duration *)&local_858);
                updateTaskType(this,*(undefined4 *)(this + 0x380));
                updateWorkStatus(this,'\\','\0');
                goto LAB_00192110;
              }
            }
                    /* try { // try from 001924a8 to 001924e7 has its CatchHandler @ 00192844 */
            updateErrorStatus(this,0x6c,0);
            updateTaskType(this,*(undefined4 *)(this + 0x380));
            goto LAB_00192100;
          }
          cVar3 = locQualityCheckAndMove(this,this[0x7ec] != (RobotDecision)0x0);
          if (cVar3 != '\0') {
            if (this[0x7ec] != (RobotDecision)0x0) {
              updateErrorStatus(this,0x7c,0);
              goto LAB_00192100;
            }
            goto LAB_001922a4;
          }
          if (this[0x732] == (RobotDecision)0x0) {
            if (this[0x734] == (RobotDecision)0x0) {
              if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
                 (iVar4 = rcutils_logging_initialize(), iVar4 != 0)) {
                fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:1982] error initializing logging: "
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
              if (local_858 != (_State_baseV2 *)0x0) {
                uVar6 = *(undefined8 *)local_858;
              }
                    /* try { // try from 001920a8 to 001920ab has its CatchHandler @ 00192850 */
              cVar3 = rcutils_logging_logger_is_enabled_for(uVar6,0x1e);
              if (local_850 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
                std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_850);
              }
              if (cVar3 != '\0') {
                    /* try { // try from 001920c8 to 001920cb has its CatchHandler @ 00192844 */
                rclcpp::Node::get_logger();
                uVar6 = 0;
                if (local_858 != (_State_baseV2 *)0x0) {
                  uVar6 = *(undefined8 *)local_858;
                }
                    /* try { // try from 001920f0 to 001920f3 has its CatchHandler @ 00192860 */
                rcutils_log(coverStartDeal()::__rcutils_logging_location,0x1e,uVar6,
                            "Localization quality is very bad!!!");
                goto LAB_001920f4;
              }
              goto LAB_00192100;
            }
            updateWorkStatus(this,'\x02','\0');
            *(undefined4 *)(this + 0x734) = 0;
          }
          else {
                    /* try { // try from 00192580 to 001927bb has its CatchHandler @ 00192844 */
            updateWorkStatus(this,'\n','\0');
          }
        }
LAB_00192110:
        if (local_848[0] != auStack_838) {
          operator_delete(local_848[0]);
        }
        goto LAB_00191b24;
      }
      *(undefined4 *)(this + 0x1558) = 0;
      *(undefined4 *)(this + 0x1559) = 0;
      if (this[0x732] != (RobotDecision)0x0) {
        updateWorkStatus(this,'\n','\0');
        *(undefined4 *)(this + 0x732) = 0;
        goto LAB_00191b24;
      }
      if (this[0x734] != (RobotDecision)0x0) {
        updateWorkStatus(this,'\x02','\0');
        *(undefined4 *)(this + 0x734) = 0;
        goto LAB_00191b24;
      }
    }
  }
  else {
    uVar6 = 0x85;
LAB_00191dc4:
    updateErrorStatus(this,uVar6,0);
  }
LAB_00191dcc:
  updateWorkStatus(this,'\x01','\0');
LAB_00191b24:
  if (local_8 - *(long *)PTR___stack_chk_guard_00448f98 != 0) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail(local_8 - *(long *)PTR___stack_chk_guard_00448f98,0);
  }
  return;
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



// ==== DONE: 7 functions
