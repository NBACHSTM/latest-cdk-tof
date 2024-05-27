"use strict";
var __defProp = Object.defineProperty;
var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
var __getOwnPropNames = Object.getOwnPropertyNames;
var __hasOwnProp = Object.prototype.hasOwnProperty;
var __export = (target, all) => {
  for (var name in all)
    __defProp(target, name, { get: all[name], enumerable: true });
};
var __copyProps = (to, from, except, desc) => {
  if (from && typeof from === "object" || typeof from === "function") {
    for (let key of __getOwnPropNames(from))
      if (!__hasOwnProp.call(to, key) && key !== except)
        __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
  }
  return to;
};
var __toCommonJS = (mod) => __copyProps(__defProp({}, "__esModule", { value: true }), mod);

// lib/iot/grafana/grafana.key-rotation.ts
var grafana_key_rotation_exports = {};
__export(grafana_key_rotation_exports, {
  handler: () => handler
});
module.exports = __toCommonJS(grafana_key_rotation_exports);
var import_aws_sdk = require("aws-sdk");
var secretName = process.env.SECRET_NAME;
var workspaceId = process.env.WORKSPACE_ID;
var secretsmanager = new import_aws_sdk.SecretsManager();
var grafana = new import_aws_sdk.Grafana();
async function handler(event) {
  console.log(JSON.stringify(event));
  const { key } = await grafana.createWorkspaceApiKey({
    workspaceId,
    keyName: "key-" + Date.now(),
    keyRole: "ADMIN",
    secondsToLive: 30 * 24 * 60 * 60
  }).promise();
  await secretsmanager.putSecretValue({
    SecretId: secretName,
    SecretString: key
  }).promise();
}
// Annotate the CommonJS export names for ESM import in node:
0 && (module.exports = {
  handler
});
