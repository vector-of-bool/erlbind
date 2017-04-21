defmodule Erlbind.Test do
  @on_load :load_nifs

  def load_nifs do
    :ok = :erlang.load_nif('./build/my-nifs', 0)
    data = test_fn 22, :dog
    require Logger
    Logger.info "Got '#{inspect data}' from the native function"
    :ok
  end

  def test_fn _, _ do
    raise "Badness"
  end
end