/*
 * Generated by asn1c-0.9.21 (http://lionet.info/asn1c)
 * From ASN.1 module "MMS"
 * 	found in "../mms-extended.asn"
 * 	`asn1c -fskeletons-copy`
 */

#ifndef	_MmsPdu_H_
#define	_MmsPdu_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ConfirmedRequestPdu.h"
#include "ConfirmedResponsePdu.h"
#include "ConfirmedErrorPDU.h"
#include "UnconfirmedPDU.h"
#include "RejectPDU.h"
#include "InitiateRequestPdu.h"
#include "InitiateResponsePdu.h"
#include "InitiateErrorPdu.h"
#include "ConcludeRequestPDU.h"
#include "ConcludeResponsePDU.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum MmsPdu_PR {
	MmsPdu_PR_NOTHING,	/* No components present */
	MmsPdu_PR_confirmedRequestPdu,
	MmsPdu_PR_confirmedResponsePdu,
	MmsPdu_PR_confirmedErrorPDU,
	MmsPdu_PR_unconfirmedPDU,
	MmsPdu_PR_rejectPDU,
	MmsPdu_PR_initiateRequestPdu,
	MmsPdu_PR_initiateResponsePdu,
	MmsPdu_PR_initiateErrorPdu,
	MmsPdu_PR_concludeRequestPDU,
	MmsPdu_PR_concludeResponsePDU
} MmsPdu_PR;

/* MmsPdu */
typedef struct MmsPdu {
	MmsPdu_PR present;
	union MmsPdu_u {
		ConfirmedRequestPdu_t	 confirmedRequestPdu;
		ConfirmedResponsePdu_t	 confirmedResponsePdu;
		ConfirmedErrorPDU_t	 confirmedErrorPDU;
		UnconfirmedPDU_t	 unconfirmedPDU;
		RejectPDU_t	 rejectPDU;
		InitiateRequestPdu_t	 initiateRequestPdu;
		InitiateResponsePdu_t	 initiateResponsePdu;
		InitiateErrorPdu_t	 initiateErrorPdu;
		ConcludeRequestPDU_t	 concludeRequestPDU;
		ConcludeResponsePDU_t	 concludeResponsePDU;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} MmsPdu_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MmsPdu;

#ifdef __cplusplus
}
#endif

#endif	/* _MmsPdu_H_ */