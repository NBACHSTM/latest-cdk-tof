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

// lib/iot/code.signingProfile.ts
var code_signingProfile_exports = {};
__export(code_signingProfile_exports, {
  handler: () => handler
});
module.exports = __toCommonJS(code_signingProfile_exports);
var import_aws_sdk = require("aws-sdk");
var signer = new import_aws_sdk.Signer();
async function handler(event) {
  console.log(JSON.stringify(event));
  const { certificateArn, codeSigningLabel, platformId } = event.ResourceProperties;
  switch (event.RequestType) {
    case "Create": {
      const profileName = "stmicro_profile_" + Date.now();
      await signer.putSigningProfile({
        platformId,
        profileName,
        signingMaterial: {
          certificateArn
        },
        signingParameters: {
          certname: codeSigningLabel
        }
      }).promise();
      return {
        PhysicalResourceId: profileName,
        Data: {
          profileName
        }
      };
    }
    case "Delete": {
      const profileName = event.PhysicalResourceId;
      await signer.cancelSigningProfile({ profileName }).promise();
      return {};
    }
    default:
  }
}
// Annotate the CommonJS export names for ESM import in node:
0 && (module.exports = {
  handler
});
