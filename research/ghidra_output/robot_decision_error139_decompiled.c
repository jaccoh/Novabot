// Functions referencing "station in map"

// ==================== RobotDecision::quitPileDeal @ 00187a00 ====================

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* RobotDecision::quitPileDeal() */

undefined1 __thiscall RobotDecision::quitPileDeal(RobotDecision *this)

{
  RobotDecision *pRVar1;
  long lVar2;
  long lVar3;
  undefined *puVar4;
  RobotDecision RVar5;
  char cVar6;
  int iVar7;
  int iVar8;
  long lVar9;
  long lVar10;
  undefined8 uVar11;
  size_t sVar12;
  undefined1 uVar13;
  double dVar14;
  double dVar15;
  double dVar16;
  double dVar17;
  double dVar18;
  double dVar19;
  int local_908;
  undefined1 local_8f0 [8];
  long local_8e8 [4];
  long local_8c8;
  undefined8 *local_8c0;
  _Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *local_8b8;
  undefined8 local_8a0;
  undefined8 uStack_898;
  undefined8 uStack_890;
  undefined8 uStack_888;
  undefined8 local_880;
  undefined8 uStack_878;
  PoseStamped_ aPStack_870 [8];
  undefined1 *local_868;
  undefined1 auStack_858 [16];
  double local_848;
  double dStack_840;
  undefined8 local_838;
  undefined8 uStack_830;
  undefined8 local_828;
  undefined8 uStack_820;
  undefined8 local_818;
  undefined1 auStack_808 [1024];
  char acStack_408 [1024];
  long local_8;
  
  local_8 = *(long *)PTR___stack_chk_guard_00448f98;
  if (this[0x602] == (RobotDecision)0x0) {
    pRVar1 = this + 0x748;
    local_8f0[0] = 1;
    uStack_898 = _UNK_0039e9c8;
    local_8a0 = _DAT_0039e9c0;
    uStack_888 = 0;
    uStack_890 = 0;
    uStack_878 = 0;
    local_880 = 0;
    lVar9 = std::chrono::_V2::steady_clock::now();
    uVar13 = 0;
    local_8c8 = lVar9;
    if (*pRVar1 == (RobotDecision)0x0) {
      dVar15 = *(double *)(this + 0xbb0);
      dVar14 = *(double *)(this + 3000);
      dVar17 = *(double *)(this + 0xbc0);
      dVar18 = *(double *)(this + 0xbc8);
      dVar16 = ((dVar15 * dVar17 - dVar14 * dVar18) * -2.0) /
               (dVar15 * dVar15 + dVar14 * dVar14 + dVar17 * dVar17 + dVar18 * dVar18);
      if (dVar16 <= DAT_0039e8a0) {
        dVar14 = atan2(dVar14,dVar15);
        dVar14 = dVar14 * -2.0;
      }
      else if (dVar16 < DAT_0039e8a8) {
        dVar16 = dVar17 * dVar18 + dVar14 * dVar15;
        dVar14 = atan2(dVar16 + dVar16,
                       ((dVar15 * dVar15 + dVar18 * dVar18) - dVar14 * dVar14) - dVar17 * dVar17);
      }
      else {
        dVar14 = atan2(dVar14,dVar15);
        dVar14 = dVar14 + dVar14;
      }
      dVar16 = *(double *)(this + 0xab0);
      dVar15 = *(double *)(this + 0xab8);
      dVar18 = *(double *)(this + 0xac0);
      dVar19 = *(double *)(this + 0xac8);
      dVar17 = ((dVar16 * dVar18 - dVar15 * dVar19) * -2.0) /
               (dVar16 * dVar16 + dVar15 * dVar15 + dVar18 * dVar18 + dVar19 * dVar19);
      if (dVar17 <= DAT_0039e8a0) {
        dVar15 = atan2(dVar15,dVar16);
        dVar15 = dVar15 * -2.0;
      }
      else if (dVar17 < DAT_0039e8a8) {
        dVar17 = dVar18 * dVar19 + dVar15 * dVar16;
        dVar15 = atan2(dVar17 + dVar17,
                       ((dVar16 * dVar16 + dVar19 * dVar19) - dVar15 * dVar15) - dVar18 * dVar18);
      }
      else {
        dVar15 = atan2(dVar15,dVar16);
        dVar15 = dVar15 + dVar15;
      }
      dVar16 = hypot(*(double *)(this + 0xb98) - *(double *)(this + 0xa98),
                     *(double *)(this + 0xba0) - *(double *)(this + 0xaa0));
      if (((DAT_0039e880 <= dVar16) || (0.5 <= ABS(dVar14 - dVar15))) ||
         (RVar5 = this[0xc48], RVar5 == (RobotDecision)0x0)) goto LAB_00187a3c;
      local_908 = 10;
    }
    else {
      if (((*pRVar1 != (RobotDecision)0x0) && (this[0x7c8] != (RobotDecision)0x0)) &&
         (this[0x7c9] == (RobotDecision)0x64)) {
        rclcpp::Clock::now();
                    /* try { // try from 00187e9c to 00187ea7 has its CatchHandler @ 001882bc */
        rclcpp::Time::operator-(&local_8c0,this + 2000);
        dVar14 = (double)rclcpp::Duration::seconds();
        rclcpp::Time::~Time(&local_8c0);
        if (600.0 < dVar14) {
          std_msgs::msg::Header_<std::allocator<void>>::Header_(aPStack_870);
          local_818 = 0x3ff0000000000000;
          dStack_840 = 0.0;
          local_848 = 0.0;
          uStack_830 = 0;
          local_838 = 0;
          uStack_820 = 0;
          local_828 = 0;
                    /* try { // try from 00187ef8 to 00187efb has its CatchHandler @ 00188288 */
          cVar6 = getChargingPose(this,aPStack_870);
          if ((cVar6 != '\0') &&
             (dVar14 = hypot(local_848 - *(double *)(this + 0xa18),
                             dStack_840 - *(double *)(this + 0xa20)), 1.0 < dVar14)) {
                    /* try { // try from 001880ec to 0018810b has its CatchHandler @ 00188288 */
            updateErrorStatus(this,0x8b,0);
                    /* try { // try from 001881d4 to 00188253 has its CatchHandler @ 00188288 */
            if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
               (iVar8 = rcutils_logging_initialize(), iVar8 != 0)) {
              fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:3798] error initializing logging: "
                     ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
              rcutils_get_error_string(auStack_808);
              rcutils_get_error_string(acStack_408);
              sVar12 = strlen(acStack_408);
              fwrite(auStack_808,1,sVar12,*(FILE **)PTR_stderr_00448e50);
              fwrite(&DAT_0036aa20,1,1,*(FILE **)PTR_stderr_00448e50);
              rcutils_reset_error();
            }
            rclcpp::Node::get_logger();
            uVar11 = 0;
            if (local_8c0 != (undefined8 *)0x0) {
              uVar11 = *local_8c0;
            }
                    /* try { // try from 0018811c to 0018811f has its CatchHandler @ 001882d0 */
            cVar6 = rcutils_logging_logger_is_enabled_for(uVar11,0x1e);
            if (local_8b8 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
              std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_8b8);
            }
            if (cVar6 != '\0') {
                    /* try { // try from 0018813c to 0018813f has its CatchHandler @ 00188288 */
              rclcpp::Node::get_logger();
              uVar11 = 0;
              if (local_8c0 != (undefined8 *)0x0) {
                uVar11 = *local_8c0;
              }
                    /* try { // try from 00188170 to 00188173 has its CatchHandler @ 00188258 */
              rcutils_log(local_848,dStack_840,*(undefined8 *)(this + 0xa18),
                          *(undefined8 *)(this + 0xa20),quitPileDeal()::__rcutils_logging_location,
                          0x1e,uVar11,"Current: %.2f %.2f    station in map:  %.2f %.2f");
              if (local_8b8 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
                std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_8b8);
              }
            }
                    /* try { // try from 00188188 to 0018818b has its CatchHandler @ 00188288 */
            rclcpp::Clock::now();
                    /* try { // try from 00188194 to 00188197 has its CatchHandler @ 00188298 */
            rclcpp::Time::operator=(this + 2000,(Time *)&local_8c0);
            rclcpp::Time::~Time(&local_8c0);
            if (local_868 != auStack_858) {
              operator_delete(local_868);
            }
            goto LAB_00187a40;
          }
          if (local_868 != auStack_858) {
            operator_delete(local_868);
          }
        }
      }
      local_908 = 0x3c;
      RVar5 = (RobotDecision)0x0;
    }
    dVar16 = *(double *)(this + 0xa98);
    dVar17 = *(double *)(this + 0xaa0);
    (**(code **)(**(long **)(this + 0x1230) + 0x20))();
    dVar14 = *(double *)(this + 0x608);
    rclcpp::Node::now();
                    /* try { // try from 00187c30 to 00187c33 has its CatchHandler @ 001882d4 */
    dVar15 = (double)rclcpp::Time::seconds();
    dVar14 = (double)((int)dVar15 % 100) / 300.0 + dVar14;
    rclcpp::Time::~Time(&local_8c0);
    if (dVar14 < 1.0) {
      dVar14 = 1.0;
    }
    else if (DAT_0039e8c0 < dVar14) {
      dVar14 = DAT_0039e8c0;
    }
    iVar8 = 0;
    do {
      lVar3 = lVar9 + 200000000;
      (**(code **)(**(long **)(this + 0x1220) + 0x20))(*(long **)(this + 0x1220),&local_8a0);
      lVar10 = std::chrono::_V2::steady_clock::now();
      if (lVar10 < lVar9) {
        local_8e8[0] = 200000000;
LAB_00187da0:
        local_8c0 = (undefined8 *)0x0;
        local_8b8 = (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0;
                    /* try { // try from 00187dac to 00187daf has its CatchHandler @ 00188290 */
        rclcpp::sleep_for(local_8e8,&local_8c0);
        lVar9 = lVar3;
        if (local_8b8 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
          std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_8b8);
        }
      }
      else {
        local_8e8[0] = lVar3 - lVar10;
        if (0 < local_8e8[0]) goto LAB_00187da0;
        lVar2 = lVar9 + 400000000;
        lVar9 = lVar10 + 200000000;
        if (lVar10 <= lVar2) {
          lVar9 = lVar3;
        }
      }
      if ((iVar8 - 2U < 2) && (*pRVar1 != (RobotDecision)0x0)) {
        if ((*PTR_g_rcutils_logging_initialized_00448fa8 == '\0') &&
           (iVar7 = rcutils_logging_initialize(), puVar4 = PTR_stderr_00448e50, iVar7 != 0)) {
          fwrite("[rcutils|/root/novabot/src/decision/compound_decision/src/robot_decision.cpp:3822] error initializing logging: "
                 ,1,0x6f,*(FILE **)PTR_stderr_00448e50);
          rcutils_get_error_string(auStack_808);
          rcutils_get_error_string(acStack_408);
          sVar12 = strlen(acStack_408);
          fwrite(auStack_808,1,sVar12,*(FILE **)puVar4);
          fwrite(&DAT_0036aa20,1,1,*(FILE **)puVar4);
          rcutils_reset_error();
        }
        rclcpp::Node::get_logger();
        uVar11 = 0;
        if (local_8c0 != (undefined8 *)0x0) {
          uVar11 = *local_8c0;
        }
                    /* try { // try from 00187e08 to 00187e0b has its CatchHandler @ 00188294 */
        cVar6 = rcutils_logging_logger_is_enabled_for(uVar11,0x1e);
        if (local_8b8 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
          std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_8b8);
        }
        if (cVar6 != '\0') {
          rclcpp::Node::get_logger();
          uVar11 = 0;
          if (local_8c0 != (undefined8 *)0x0) {
            uVar11 = *local_8c0;
          }
                    /* try { // try from 00187fcc to 00187fcf has its CatchHandler @ 00188270 */
          rcutils_log(quitPileDeal()::__rcutils_logging_location,0x1e,uVar11,
                      "Robot is steal in charging status, try release lock again!!!");
          if (local_8b8 != (_Sp_counted_base<(__gnu_cxx::_Lock_policy)2> *)0x0) {
            std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release(local_8b8);
          }
        }
        (**(code **)(**(long **)(this + 0x1230) + 0x20))(*(long **)(this + 0x1230),local_8f0);
      }
      if ((PTR___pthread_key_create_00448e10 != (undefined *)0x0) &&
         (iVar7 = pthread_mutex_lock((pthread_mutex_t *)(this + 0x570)), iVar7 != 0)) {
                    /* WARNING: Subroutine does not return */
        std::__throw_system_error(iVar7);
      }
      dVar15 = hypot(*(double *)(this + 0xa98) - dVar16,*(double *)(this + 0xaa0) - dVar17);
      if (PTR___pthread_key_create_00448e10 != (undefined *)0x0) {
        pthread_mutex_unlock((pthread_mutex_t *)(this + 0x570));
      }
      if (dVar14 < dVar15) {
        local_8a0 = 0;
        (**(code **)(**(long **)(this + 0x1220) + 0x20))(*(long **)(this + 0x1220),&local_8a0);
        RVar5 = (RobotDecision)(*pRVar1 == (RobotDecision)0x0);
        *(uint *)(this + 0x6e5) = (uint)(8 < iVar8);
        break;
      }
      updateWorkStatus(this,(uchar)this[900],(uchar)this[0x385]);
      if ((this[0x732] != (RobotDecision)0x0) || (this[0x734] != (RobotDecision)0x0))
      goto LAB_00187a40;
      iVar8 = iVar8 + 1;
    } while (local_908 != iVar8);
    if (RVar5 == (RobotDecision)0x0) {
      updateErrorStatus(this,0x75,0);
      uVar13 = 0;
      goto LAB_00187a40;
    }
  }
LAB_00187a3c:
  uVar13 = 1;
LAB_00187a40:
  if (local_8 - *(long *)PTR___stack_chk_guard_00448f98 != 0) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail(local_8 - *(long *)PTR___stack_chk_guard_00448f98,0);
  }
  return uVar13;
}


