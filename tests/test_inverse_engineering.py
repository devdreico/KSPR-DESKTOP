import unittest

from backend.kspr_engine.inverse_engineering import InverseEngineeringRequest, reconstruct


class InverseEngineeringTests(unittest.TestCase):
    def test_reconstruction_is_traceable_and_domain_agnostic(self):
        result = reconstruct(InverseEngineeringRequest(
            subject="Sistema de riego",
            domain="physical",
            objective="Reconstruir componentes y secuencia operacional",
            evidence=[{
                "id": "obs-1",
                "kind": "observation",
                "title": "Observación de campo",
                "content": "# Depósito\n- Bomba\nPaso: abrir válvula",
            }],
        ))
        self.assertEqual(result.status, "completed")
        self.assertGreaterEqual(len(result.nodes), 3)
        self.assertTrue(result.relations)
        self.assertTrue(all(node.evidence_ids for node in result.nodes))
        self.assertTrue(result.hypotheses[0].supporting_evidence)

    def test_requires_evidence(self):
        with self.assertRaises(ValueError):
            InverseEngineeringRequest(subject="Idea sin evidencia", domain="imaginary", evidence=[])


if __name__ == "__main__":
    unittest.main()
