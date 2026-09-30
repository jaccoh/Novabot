// Decompile the recharge and error-status handlers of the stock robot_decision
// (mower firmware v6.0.2, install/compound_decision/lib/compound_decision/robot_decision).
//
//   analyzeHeadless <proj_dir> robotdecision \
//     -import research/firmware/mower_firmware_v6.0.2/install/compound_decision/lib/compound_decision/robot_decision \
//     -scriptPath research/ghidra-scripts -postScript DecompRobotDecisionRecharge.java <output.c>
//@category Novabot
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.FunctionManager;
import ghidra.util.task.ConsoleTaskMonitor;
import java.io.PrintWriter;

public class DecompRobotDecisionRecharge extends GhidraScript {
  public void run() throws Exception {
    String[] args = getScriptArgs();
    String out = args.length > 0 ? args[0] : "robot_decision_recharge_decompiled.c";
    String[] names = {"rechargeDeal", "handleAutoRechargeTask", "handleNavToRechargeTask", "handleCancelRechargeTask",
                      "updateErrorStatus", "rechargeFinishedDeal", "handleRechargeResult", "coverStartDeal", "coverContinueDeal"};
    FunctionManager fm = currentProgram.getFunctionManager();
    DecompInterface decomp = new DecompInterface();
    decomp.openProgram(currentProgram);
    PrintWriter w = new PrintWriter(out, "UTF-8");
    w.println("// Ghidra decompile of robot_decision (mower fw v6.0.2): recharge + error-status handlers");
    FunctionIterator it = fm.getFunctions(true);
    int n = 0;
    while (it.hasNext()) {
      Function f = it.next();
      String full = f.getName(true);
      if (!full.startsWith("RobotDecision::")) continue;
      boolean m = false;
      for (String s : names) { if (full.contains(s)) { m = true; break; } }
      if (!m) continue;
      w.println("\n// ==================== " + full + " @ " + f.getEntryPoint() + " ====================");
      DecompileResults res = decomp.decompileFunction(f, 300, new ConsoleTaskMonitor());
      if (res != null && res.decompileCompleted()) w.println(res.getDecompiledFunction().getC());
      else w.println("// decompile FAILED: " + (res == null ? "null" : res.getErrorMessage()));
      n++;
      println("decompiled " + full);
    }
    w.println("\n// ==== DONE: " + n + " functions");
    w.close();
    println("==== DONE DecompRobotDecisionRecharge: " + n + " functions -> " + out);
  }
}
